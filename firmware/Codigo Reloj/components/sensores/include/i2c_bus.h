#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "sensor_config.h"

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t init_i2c(void);
#ifdef __cplusplus
}
#endif
