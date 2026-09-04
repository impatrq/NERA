/**
 * @file event_bus.h
 * @brief Bus de eventos inter-módulo para NERA.
 *
 * ¿Qué es un Event Bus (bus de eventos)?
 * Es un mecanismo de comunicación que permite que distintos módulos del
 * firmware se comuniquen sin conocerse directamente entre sí.
 *
 * Sin event bus:
 *   sensor_task llama directamente a ui_update_bpm(72.0)
 *   → El sensor conoce la UI. Si la UI cambia, el sensor debe modificarse.
 *
 * Con event bus:
 *   sensor_task publica: NERA_EVENT_HEART_RATE_UPDATED, data={bpm: 72.0}
 *   ui_task escucha: NERA_EVENT_HEART_RATE_UPDATED → actualiza la pantalla
 *   → Sensor y UI son independientes. No se conocen entre sí.
 *
 * Esto hace el código más modular, testeable y fácil de mantener.
 *
 * NOTA: En esta Etapa 1A el event bus es un stub (esqueleto vacío).
 * Se implementará completamente en la Etapa 1C cuando integremos LVGL
 * y necesitemos comunicación real entre la tarea de sensores y la de UI.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TIPOS DE EVENTOS
// =============================================================================

/**
 * @brief Identificadores de eventos del sistema NERA.
 *
 * Cada módulo tiene un rango de IDs para evitar colisiones:
 *   0x0000 - 0x00FF : Eventos del sistema
 *   0x0100 - 0x01FF : Eventos de sensores
 *   0x0200 - 0x02FF : Eventos de UI
 *   0x0300 - 0x03FF : Eventos de BLE
 *   0x0400 - 0x04FF : Eventos de storage
 */
typedef enum {
    // --- Eventos del sistema ---
    NERA_EVENT_SYSTEM_READY         = 0x0001, ///< El sistema terminó de inicializarse
    NERA_EVENT_MODE_CHANGED         = 0x0002, ///< Cambió el modo de operación
    NERA_EVENT_LOW_BATTERY          = 0x0003, ///< Batería baja detectada
    NERA_EVENT_CRITICAL_BATTERY     = 0x0004, ///< Batería crítica (< 5%)

    // --- Eventos de sensores ---
    NERA_EVENT_HEART_RATE_UPDATED   = 0x0101, ///< Nuevas medición de BPM disponible
    NERA_EVENT_TEMPERATURE_UPDATED  = 0x0102, ///< Nueva temperatura disponible
    NERA_EVENT_BATTERY_UPDATED      = 0x0103, ///< Nuevo nivel de batería disponible
    NERA_EVENT_SENSOR_ERROR         = 0x0104, ///< Un sensor falló

    // --- Eventos de UI ---
    NERA_EVENT_TOUCH_TAP            = 0x0201, ///< Toque simple detectado
    NERA_EVENT_TOUCH_SWIPE_LEFT     = 0x0202, ///< Deslizamiento hacia la izquierda
    NERA_EVENT_TOUCH_SWIPE_RIGHT    = 0x0203, ///< Deslizamiento hacia la derecha
    NERA_EVENT_TOUCH_SWIPE_UP       = 0x0204, ///< Deslizamiento hacia arriba
    NERA_EVENT_TOUCH_SWIPE_DOWN     = 0x0205, ///< Deslizamiento hacia abajo
    NERA_EVENT_SCREEN_DIM           = 0x0206, ///< Pantalla atenuándose
    NERA_EVENT_SCREEN_OFF           = 0x0207, ///< Pantalla apagándose
    NERA_EVENT_SCREEN_ON            = 0x0208, ///< Pantalla encendiéndose

    // --- Eventos de BLE ---
    NERA_EVENT_BLE_CONNECTED        = 0x0301, ///< Dispositivo BLE conectado
    NERA_EVENT_BLE_DISCONNECTED     = 0x0302, ///< Dispositivo BLE desconectado
    NERA_EVENT_TIME_SYNCED          = 0x0303, ///< Hora sincronizada via BLE/WiFi

} NeraEventId;

/**
 * @brief Datos opcionales que acompañan a un evento.
 *
 * Usa un union para que distintos tipos de eventos puedan transportar
 * diferentes tipos de datos sin desperdiciar memoria.
 * Union = comparte la misma memoria para todos los campos.
 */
typedef union {
    float   float_val;       ///< Valor decimal (ej: BPM, temperatura)
    int32_t int_val;         ///< Valor entero (ej: porcentaje batería)
    uint8_t bytes[8];        ///< Datos arbitrarios
} NeraEventData;

/**
 * @brief Estructura completa de un evento.
 */
typedef struct {
    NeraEventId   id;     ///< Qué tipo de evento es
    NeraEventData data;   ///< Datos asociados al evento
    int64_t       timestamp_us; ///< Cuándo ocurrió (microsegundos desde boot)
} NeraEvent;

// =============================================================================
// FUNCIONES PÚBLICAS
// =============================================================================

/**
 * @brief Inicializa el bus de eventos.
 * Crea la queue de FreeRTOS para los eventos.
 * @return ESP_OK si fue exitoso.
 */
esp_err_t event_bus_init(void);

/**
 * @brief Publica un evento en el bus.
 *
 * Cualquier módulo puede publicar eventos. Si la queue está llena,
 * el evento más antiguo se descarta (política "overwrite").
 *
 * Esta función es segura para llamar desde interrupciones ISR.
 *
 * @param id   Tipo de evento.
 * @param data Datos del evento (puede ser NULL).
 * @return ESP_OK si el evento fue publicado exitosamente.
 */
esp_err_t event_bus_publish(NeraEventId id, const NeraEventData *data);

/**
 * @brief Espera y obtiene el próximo evento.
 *
 * Bloquea la tarea actual hasta que haya un evento disponible o
 * hasta que expire el timeout.
 *
 * @param[out] event     Puntero donde se guardará el evento recibido.
 * @param      timeout_ms Tiempo máximo de espera en ms. 0 = sin espera.
 *                        portMAX_DELAY = esperar indefinidamente.
 * @return ESP_OK si se recibió un evento, ESP_ERR_TIMEOUT si expiró.
 */
esp_err_t event_bus_receive(NeraEvent *event, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
