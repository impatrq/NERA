#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t ble_manager_init(void);
bool ble_manager_is_ready(void);
void ble_manager_task(void *arg);
