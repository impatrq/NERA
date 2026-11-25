#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/i2c.h"
#include "esp_err.h"

// IMPORTANTE: mismo bus externo I2C1, SDA21 y SCL2; el tactil usa otro bus.
// No cambiar estos valores para modificar la apariencia del reloj.
#define MLX_MPU6050_I2C_PORT     I2C_NUM_1
#define MLX_MPU6050_SDA          21
#define MLX_MPU6050_SCL          2
