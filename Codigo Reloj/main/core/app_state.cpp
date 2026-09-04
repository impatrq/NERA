/**
 * @file app_state.cpp
 * @brief Implementación del estado global de la aplicación NERA.
 *
 * Este archivo implementa las funciones declaradas en app_state.h.
 * 
 * El acceso al estado siempre pasa por un mutex de FreeRTOS para evitar
 * "race conditions" (condiciones de carrera).
 *
 * ¿Qué es una race condition?
 * Imagina que la tarea de sensores actualiza el BPM al mismo tiempo que
 * la tarea de UI lo está leyendo para mostrarlo en pantalla. Si ambas
 * operaciones ocurren exactamente al mismo instante (en el mismo dato en
 * memoria), el resultado es impredecible — podría leerse un valor corrupto.
 * El mutex previene esto: solo una tarea accede al dato a la vez.
 */

#include "app_state.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "utils/logger.h"
#include "sdkconfig.h"

// Tag de logging específico para este módulo.
static const char *TAG = NERA_TAG_STATE;

// =============================================================================
// ESTADO INTERNO
// =============================================================================

// Estado global del sistema. Es una variable estática (no global en el sentido
// C), lo que significa que solo es accesible desde este archivo.
// El acceso externo siempre va por las funciones públicas.
static NeraAppState s_state;

// Mutex de FreeRTOS que protege el acceso al estado.
// SemaphoreHandle_t es el tipo de FreeRTOS para mutex y semáforos.
static SemaphoreHandle_t s_mutex = NULL;

// Indica si el sistema fue inicializado.
static bool s_initialized = false;

// =============================================================================
// FUNCIONES PRIVADAS (helper)
// =============================================================================

static void append_history_value(float *history, uint8_t *count, float value)
{
    if (history == NULL || count == NULL) {
        return;
    }

    if (*count < NERA_HISTORY_BUFFER_SIZE) {
        history[*count] = value;
        (*count)++;
        return;
    }

    memmove(history, history + 1, (NERA_HISTORY_BUFFER_SIZE - 1) * sizeof(float));
    history[NERA_HISTORY_BUFFER_SIZE - 1] = value;
}

/**
 * @brief Toma el mutex con timeout.
 * 
 * xSemaphoreTake() bloquea la tarea actual hasta obtener el mutex.
 * El timeout evita que una tarea quede bloqueada indefinidamente.
 * pdMS_TO_TICKS() convierte milisegundos a "ticks" de FreeRTOS.
 * 
 * @return true si se obtuvo el mutex, false si hubo timeout.
 */
static inline bool state_lock(void) {
    if (s_mutex == NULL) return false;
    return xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100)) == pdTRUE;
}

/**
 * @brief Libera el mutex.
 * SIEMPRE debe llamarse después de state_lock() exitoso.
 */
static inline void state_unlock(void) {
    xSemaphoreGive(s_mutex);
}

// =============================================================================
// IMPLEMENTACIÓN DE FUNCIONES PÚBLICAS
// =============================================================================

esp_err_t app_state_init(void) {
    if (s_initialized) {
        NERA_LOGW(TAG, "Estado ya inicializado, ignorando segunda inicialización");
        return ESP_OK;
    }

    // Crear el mutex de FreeRTOS.
    // xSemaphoreCreateMutex() crea un mutex que puede tomarse y liberarse
    // desde diferentes tareas.
    s_mutex = xSemaphoreCreateMutex();
    if (s_mutex == NULL) {
        NERA_LOGE(TAG, "No se pudo crear el mutex de estado global");
        return ESP_ERR_NO_MEM;
    }

    // Inicializar el estado con valores por defecto.
    // memset a 0 garantiza que todos los campos boolean sean false
    // y todos los numéricos sean 0.
    memset(&s_state, 0, sizeof(NeraAppState));

    // Establecer valores iniciales específicos.
    s_state.mode          = NERA_MODE_BOOT;
    s_state.ble_state     = NERA_BLE_DISABLED;
    s_state.heart_history_count = 0;
    s_state.temp_history_count  = 0;
    s_state.ui_ready      = false;

    // ¿Usar sensores mock? Configurado en menuconfig.
    // CONFIG_NERA_USE_MOCK_SENSORS es definido por Kconfig según la selección
    // del usuario en "idf.py menuconfig".
#if CONFIG_NERA_USE_MOCK_SENSORS
    s_state.use_mocks = true;
#else
    s_state.use_mocks = false;
#endif

    // Valores iniciales del estado de batería.
    s_state.battery.percentage = 100;
    s_state.battery.state      = NERA_BATTERY_UNKNOWN;

    // Hora por defecto (se actualizará cuando el sistema obtenga la hora real).
    s_state.datetime.year    = 2026;
    s_state.datetime.month   = 9;
    s_state.datetime.day     = 1;
    s_state.datetime.hour    = 0;
    s_state.datetime.minute  = 0;
    s_state.datetime.second  = 0;
    s_state.datetime.weekday = 2; // Martes
    s_state.datetime.synced  = false;

    s_initialized = true;
    NERA_LOGI(TAG, "Estado global inicializado. Modo: BOOT. Mocks: %s",
              s_state.use_mocks ? "ACTIVOS" : "DESACTIVADOS");

    return ESP_OK;
}

esp_err_t app_state_get(NeraAppState *state) {
    if (state == NULL) return ESP_ERR_INVALID_ARG;
    if (!s_initialized) return ESP_ERR_INVALID_STATE;

    if (!state_lock()) {
        NERA_LOGW(TAG, "Timeout al obtener mutex para app_state_get");
        return ESP_ERR_TIMEOUT;
    }

    // Copia atómica del estado completo.
    memcpy(state, &s_state, sizeof(NeraAppState));

    state_unlock();
    return ESP_OK;
}

void app_state_set_mode(NeraSystemMode mode) {
    if (!state_lock()) return;
    
    NERA_LOGI(TAG, "Modo cambiando: %d -> %d", (int)s_state.mode, (int)mode);
    s_state.mode = mode;
    
    state_unlock();
}

void app_state_set_heart(const NeraHeartData *data) {
    if (data == NULL || !state_lock()) return;
    memcpy(&s_state.heart, data, sizeof(NeraHeartData));
    append_history_value(s_state.heart_history, &s_state.heart_history_count, data->bpm);
    state_unlock();
}

void app_state_set_temp(const NeraTempData *data) {
    if (data == NULL || !state_lock()) return;
    memcpy(&s_state.temp, data, sizeof(NeraTempData));
    append_history_value(s_state.temp_history, &s_state.temp_history_count, data->celsius);
    state_unlock();
}

void app_state_set_battery(const NeraBatteryData *data) {
    if (data == NULL || !state_lock()) return;
    memcpy(&s_state.battery, data, sizeof(NeraBatteryData));
    state_unlock();
}

void app_state_set_datetime(const NeraDateTime *dt) {
    if (dt == NULL || !state_lock()) return;
    memcpy(&s_state.datetime, dt, sizeof(NeraDateTime));
    state_unlock();
}
