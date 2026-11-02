/** Politica de energia para DisplayManager. Conserva el brillo elegido mientras
 * aplica una atenuacion temporal. No implementa deep sleep ni controla sensores. */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t power_manager_init(void);
esp_err_t power_manager_set_saver(bool enabled);
bool power_manager_is_saver_enabled(void);
uint8_t power_manager_get_brightness(void);
esp_err_t power_manager_set_brightness(uint8_t value);
void power_manager_tick(uint32_t inactive_ms);
