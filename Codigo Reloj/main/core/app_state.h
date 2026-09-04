/**
 * @file app_state.h
 * @brief Estado global de la aplicación NERA.
 *
 * AppState centraliza el estado de todo el sistema en una única estructura.
 * 
 * ¿Por qué centralizar el estado?
 * En un firmware con múltiples tareas (UI, sensores, BLE, storage), cada
 * tarea necesita acceder a datos compartidos: ¿cuánto BPM hay ahora?,
 * ¿qué porcentaje de batería queda?, ¿está conectado BLE?
 * 
 * Sin centralización, cada módulo mantendría su propio estado y sería
 * difícil mantener consistencia. Con AppState, todos leen y escriben
 * en el mismo lugar, protegido por un mutex de FreeRTOS.
 *
 * ¿Qué es un mutex?
 * Mutex = Mutual Exclusion. Es un mecanismo de sincronización que evita
 * que dos tareas modifiquen el mismo dato al mismo tiempo (condición de
 * carrera o "race condition"). Solo una tarea puede "tomar" el mutex a la vez.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "nera_config.h"

#ifdef __cplusplus
extern "C" {
#endif
// =============================================================================
// ENUMERACIONES DE ESTADO
// =============================================================================

/**
 * @brief Modos de operación del sistema.
 * El sistema siempre está en exactamente uno de estos modos.
 */
typedef enum {
    NERA_MODE_BOOT          = 0,  ///< Sistema arrancando, UI no disponible
    NERA_MODE_INITIALIZING  = 1,  ///< Inicializando módulos (splash screen)
    NERA_MODE_READY         = 2,  ///< Listo para interacción normal
    NERA_MODE_MEASURING     = 3,  ///< Midiendo activamente un sensor
    NERA_MODE_SLEEP         = 4,  ///< Pantalla apagada, bajo consumo
    NERA_MODE_ERROR         = 5,  ///< Error crítico del sistema
    NERA_MODE_LOW_BATTERY   = 6,  ///< Batería baja (< 10%)
    NERA_MODE_CHARGING      = 7,  ///< Cargando batería
} NeraSystemMode;

/**
 * @brief Estado de conexión BLE.
 */
typedef enum {
    NERA_BLE_DISCONNECTED   = 0,  ///< Sin dispositivo conectado
    NERA_BLE_ADVERTISING    = 1,  ///< Anunciando presencia (buscable)
    NERA_BLE_CONNECTED      = 2,  ///< Conectado a un smartphone
    NERA_BLE_DISABLED       = 3,  ///< BLE apagado para ahorrar energía
} NeraBLEState;

/**
 * @brief Estado de la batería.
 */
typedef enum {
    NERA_BATTERY_UNKNOWN      = 0, ///< No se puede determinar el estado
    NERA_BATTERY_DISCHARGING  = 1, ///< Descargándose normalmente
    NERA_BATTERY_CHARGING     = 2, ///< Cargando
    NERA_BATTERY_FULL         = 3, ///< Completamente cargada
    NERA_BATTERY_LOW          = 4, ///< Nivel bajo (< 20%)
    NERA_BATTERY_CRITICAL     = 5, ///< Nivel crítico (< 5%)
} NeraBatteryState;

// =============================================================================
// ESTRUCTURA DE ESTADO GLOBAL
// =============================================================================

/**
 * @brief Datos del sensor de frecuencia cardíaca.
 */
typedef struct {
    float   bpm;            ///< BPM actual (latidos por minuto)
    float   bpm_min;        ///< BPM mínimo registrado en la sesión
    float   bpm_max;        ///< BPM máximo registrado en la sesión
    float   bpm_avg;        ///< Promedio de BPM en la sesión
    bool    is_valid;       ///< true si la lectura actual es confiable
    bool    sensor_ok;      ///< true si el sensor está disponible y funcionando
    int64_t last_update_us; ///< Timestamp de la última actualización (microsegundos)
} NeraHeartData;

/**
 * @brief Datos del sensor de temperatura.
 */
typedef struct {
    float   celsius;        ///< Temperatura en grados Celsius
    float   celsius_min;    ///< Mínima registrada en la sesión
    float   celsius_max;    ///< Máxima registrada en la sesión
    bool    is_valid;       ///< true si la lectura es confiable
    bool    sensor_ok;      ///< true si el sensor está disponible
    int64_t last_update_us; ///< Timestamp de la última actualización
} NeraTempData;

/**
 * @brief Datos de la batería.
 */
typedef struct {
    uint8_t         percentage;   ///< Porcentaje (0-100)
    float           voltage_mv;   ///< Voltaje en milivolts
    NeraBatteryState state;       ///< Estado actual de la batería
    bool            sensor_ok;    ///< true si el ADC de batería funciona
} NeraBatteryData;

/**
 * @brief Fecha y hora del sistema.
 */
typedef struct {
    uint16_t year;
    uint8_t  month;   ///< 1-12
    uint8_t  day;     ///< 1-31
    uint8_t  hour;    ///< 0-23
    uint8_t  minute;  ///< 0-59
    uint8_t  second;  ///< 0-59
    uint8_t  weekday; ///< 0=Domingo, 1=Lunes, ..., 6=Sábado
    bool     synced;  ///< true si la hora fue sincronizada (BLE/WiFi/RTC)
} NeraDateTime;

/**
 * @brief Estado global completo del sistema NERA.
 *
 * Esta estructura contiene toda la información de estado accesible desde
 * cualquier módulo del firmware. Siempre se accede a través de las funciones
 * app_state_get() y app_state_set() que garantizan acceso seguro con mutex.
 */
typedef struct {
    NeraSystemMode  mode;                  ///< Modo de operación actual
    NeraBLEState    ble_state;             ///< Estado de BLE
    NeraHeartData   heart;                 ///< Datos del corazón/BPM
    NeraTempData    temp;                  ///< Datos de temperatura
    NeraBatteryData battery;               ///< Datos de batería
    NeraDateTime    datetime;              ///< Fecha y hora actual
    float           heart_history[NERA_HISTORY_BUFFER_SIZE];
    float           temp_history[NERA_HISTORY_BUFFER_SIZE];
    uint8_t         heart_history_count;   ///< Cantidad de muestras válidas del historial
    uint8_t         temp_history_count;    ///< Cantidad de muestras válidas del historial
    bool            ui_ready;              ///< true cuando LVGL está inicializado
    bool            use_mocks;             ///< true si se usan sensores simulados
} NeraAppState;

// =============================================================================
// FUNCIONES PÚBLICAS
// =============================================================================

/**
 * @brief Inicializa el sistema de estado global.
 *
 * Crea el mutex de FreeRTOS y establece los valores por defecto.
 * DEBE llamarse antes de cualquier otra función de app_state.
 *
 * @return ESP_OK si la inicialización fue exitosa.
 */
esp_err_t app_state_init(void);

/**
 * @brief Obtiene una copia del estado global.
 *
 * Esta función toma el mutex, copia el estado y lo libera.
 * La copia es segura para leer desde cualquier tarea.
 *
 * @param[out] state Puntero donde se copiará el estado.
 * @return ESP_OK si la operación fue exitosa.
 */
esp_err_t app_state_get(NeraAppState *state);

/**
 * @brief Actualiza el modo de operación del sistema.
 * @param mode Nuevo modo a establecer.
 */
void app_state_set_mode(NeraSystemMode mode);

/**
 * @brief Actualiza los datos de frecuencia cardíaca.
 * @param data Nuevos datos del sensor cardíaco.
 */
void app_state_set_heart(const NeraHeartData *data);

/**
 * @brief Actualiza los datos de temperatura.
 * @param data Nuevos datos del sensor de temperatura.
 */
void app_state_set_temp(const NeraTempData *data);

/**
 * @brief Actualiza los datos de batería.
 * @param data Nuevos datos de batería.
 */
void app_state_set_battery(const NeraBatteryData *data);

/**
 * @brief Actualiza la fecha y hora del sistema.
 * @param dt Nueva fecha y hora.
 */
void app_state_set_datetime(const NeraDateTime *dt);

#ifdef __cplusplus
}
#endif
