/**
 * @file event_bus.cpp
 * @brief Implementación mínima del bus de eventos (stub para Etapa 1A).
 *
 * En esta etapa, el event bus solo implementa la inicialización básica.
 * La funcionalidad completa de publicación y recepción de eventos se
 * activará en etapas posteriores cuando tengamos módulos que necesiten
 * comunicarse entre sí.
 */

#include "event_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "utils/logger.h"

static const char *TAG = NERA_TAG_STATE;

// Tamaño de la queue de eventos (cuántos eventos puede contener antes de
// necesitar ser procesada).
#define EVENT_BUS_QUEUE_SIZE    16

// Queue de FreeRTOS que almacena los eventos pendientes de procesar.
// QueueHandle_t es el tipo de FreeRTOS para queues (colas FIFO).
static QueueHandle_t s_event_queue = NULL;

static bool s_initialized = false;

esp_err_t event_bus_init(void) {
    if (s_initialized) {
        return ESP_OK;
    }

    // Crear una queue de FreeRTOS.
    // xQueueCreate(número_de_slots, tamaño_de_cada_item)
    // Cada "item" es un NeraEvent completo.
    s_event_queue = xQueueCreate(EVENT_BUS_QUEUE_SIZE, sizeof(NeraEvent));
    if (s_event_queue == NULL) {
        NERA_LOGE(TAG, "No se pudo crear la queue del event bus");
        return ESP_ERR_NO_MEM;
    }

    s_initialized = true;
    NERA_LOGI(TAG, "Event bus inicializado (queue: %d slots)", EVENT_BUS_QUEUE_SIZE);
    return ESP_OK;
}

esp_err_t event_bus_publish(NeraEventId id, const NeraEventData *data) {
    if (!s_initialized || s_event_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    NeraEvent event = {};
    event.id           = id;
    event.timestamp_us = esp_timer_get_time();
    if (data != NULL) {
        event.data = *data;
    }

    // xQueueSend() agrega el evento al final de la queue.
    // Si la queue está llena, espera 0 ticks (no bloquea).
    if (xQueueSend(s_event_queue, &event, 0) != pdTRUE) {
        NERA_LOGW(TAG, "Event bus lleno, evento 0x%04X descartado", (int)id);
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t event_bus_receive(NeraEvent *event, uint32_t timeout_ms) {
    if (event == NULL || !s_initialized) {
        return ESP_ERR_INVALID_ARG;
    }

    // Convertir ms a ticks de FreeRTOS.
    // portMAX_DELAY = esperar indefinidamente.
    TickType_t ticks = (timeout_ms == UINT32_MAX) 
                       ? portMAX_DELAY 
                       : pdMS_TO_TICKS(timeout_ms);

    if (xQueueReceive(s_event_queue, event, ticks) == pdTRUE) {
        return ESP_OK;
    }

    return ESP_ERR_TIMEOUT;
}
