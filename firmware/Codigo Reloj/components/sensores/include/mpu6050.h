#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/i2c.h"
#include "esp_err.h"

// FUNCIONAL: registros, datos y API de movimiento; valores originales.
// MPU6050_INT se conserva por compatibilidad; no habilita una interrupcion.
#define MPU6050_INT              27
#define MPU6050_I2C_ADDR         0x68
#define MPU6050_WHO_AM_I_REG     0x75
#define MPU6050_PWR_MGMT_1_REG   0x6B
#define MPU6050_PWR_MGMT_2_REG   0x6C
#define MPU6050_ACCEL_XOUT_H     0x3B
#define MPU6050_GYRO_XOUT_H      0x43
#define MPU6050_ACCEL_CONFIG     0x1C
#define MPU6050_MOT_THR          0x1F
#define MPU6050_MOT_DUR          0x20
#define MPU6050_INT_ENABLE       0x38
#define MPU6050_INT_PIN_CFG      0x37
#define MPU6050_INT_STATUS       0x3A
#define MPU6050_THRESHOLD 20

typedef struct {
    int16_t ax;
    int16_t ay; 
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} mpu6050_data_t;

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t mpu6050_init(i2c_port_t i2c_num);
esp_err_t mpu6050_read(mpu6050_data_t *data, i2c_port_t i2c_num);
esp_err_t mpu6050_enable_wom(i2c_port_t i2c_num, uint8_t threshold_val); // funcion para activar el modo Wake-On-Motion
esp_err_t mpu6050_clear_int(i2c_port_t i2c_num); // funcion para limpiar el pin INT del MPU6050
#ifdef __cplusplus
}
#endif
