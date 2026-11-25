#pragma once

#include <stdint.h>
#include "esp_err.h"

// Paquete v1 de 20 bytes, tambien es el formato del anillo en Flash.
// epoch_seconds=0 significa que no se pudo establecer la hora de esa muestra.
struct HistoryPoint {
    uint32_t sequence;
    uint32_t epoch_seconds;
    uint32_t uptime_seconds;
    int16_t bpm;                // 0: sin pulso valido.
    int16_t object_deci_c;      // INT16_MIN: sin temperatura valida.
    uint32_t checksum;
};
static_assert(sizeof(HistoryPoint) == 20);

esp_err_t history_service_init(void);
bool history_service_is_ready(void);
void history_service_task(void *);
// La tarea BLE llama esto luego de sincronizar la hora y antes de transferir.
void history_service_backfill_current_boot(void);
bool history_service_get_after(uint32_t sequence, HistoryPoint *out);
uint32_t history_service_latest_sequence(void);
uint32_t history_service_generation(void);
