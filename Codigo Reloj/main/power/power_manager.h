#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t power_manager_init(void);
esp_err_t power_manager_set_saver(bool enabled);
bool power_manager_is_saver_enabled(void);
uint8_t power_manager_get_brightness(void);
