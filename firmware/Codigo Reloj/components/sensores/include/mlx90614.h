#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/i2c.h"
#include "esp_err.h"

// FUNCIONAL: temperaturas de objeto y ambiente; no son temperatura clinica.
#define MLX90614_I2C_ADDR        0x5A

typedef struct {
    float mlx_amb_temp;
    float mlx_object_temp;
} mlx90614_data_t;

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t mlx90614_read(mlx90614_data_t *data, i2c_port_t i2c_num);
#ifdef __cplusplus
}
#endif
