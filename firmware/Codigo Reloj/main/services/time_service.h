#pragma once

#include <stdint.h>
#include "esp_err.h"

// UTC recibido del telefono. La zona horaria se usa solo al mostrar la hora.
esp_err_t time_service_sync(uint32_t epoch_seconds, int16_t utc_offset_minutes);
bool time_service_is_synced(void);
uint32_t time_service_epoch(void);
int16_t time_service_utc_offset_minutes(void);
bool time_service_local_hhmm(char out[6]);
// Permite fechar muestras tomadas antes de la primera sincronizacion de este arranque.
uint32_t time_service_epoch_for_uptime(int64_t uptime_us);
