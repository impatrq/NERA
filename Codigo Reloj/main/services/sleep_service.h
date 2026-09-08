#pragma once

#include <stdbool.h>
#include "esp_err.h"

typedef struct {
    uint16_t duration_minutes;
    uint8_t quality_percent;
    uint16_t light_minutes;
    uint16_t deep_minutes;
    uint16_t awake_minutes;
    bool is_estimate;
} NeraSleepData;

esp_err_t sleep_service_init(void);
esp_err_t sleep_service_get_data(NeraSleepData *data);
