// FUNCIONAL: implementacion trasladada sin cambiar las transacciones I2C.
#include "mpu6050.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MPUMLX";
#define I2C_TIMEOUT_TICKS pdMS_TO_TICKS(100)

static esp_err_t mpu6050_write_byte(i2c_port_t port, uint8_t reg, uint8_t value) {
    const uint8_t bytes[2] = {reg, value};
    return i2c_master_write_to_device(port, MPU6050_I2C_ADDR, bytes, sizeof(bytes), I2C_TIMEOUT_TICKS);
}

static esp_err_t mpu6050_read_bytes(i2c_port_t port, uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(port, MPU6050_I2C_ADDR, &reg, 1, data, len, I2C_TIMEOUT_TICKS);
}

esp_err_t mpu6050_init(i2c_port_t port) {
    uint8_t identity = 0;
    esp_err_t ret = mpu6050_read_bytes(port, MPU6050_WHO_AM_I_REG, &identity, 1);
    if (ret != ESP_OK) return ret;
    if (identity != 0x68 && identity != 0x70) {
        ESP_LOGW(TAG, "WHO_AM_I inesperado: 0x%02X", identity);
        return ESP_FAIL;
    }
    // Sale de sleep. Los rangos por defecto son +/-2 g y +/-250 grados/s.
    ret = mpu6050_write_byte(port, MPU6050_PWR_MGMT_1_REG, 0x00);
    if (ret == ESP_OK) ESP_LOGI(TAG, "MPU6050 listo (WHO_AM_I=0x%02X)", identity);
    return ret;
}

esp_err_t mpu6050_read(mpu6050_data_t *data, i2c_port_t port) {
    if (!data) return ESP_ERR_INVALID_ARG;
    uint8_t bytes[14];
    esp_err_t ret = mpu6050_read_bytes(port, MPU6050_ACCEL_XOUT_H, bytes, sizeof(bytes));
    if (ret != ESP_OK) return ret;
    data->ax = (int16_t)((bytes[0] << 8) | bytes[1]);
    data->ay = (int16_t)((bytes[2] << 8) | bytes[3]);
    data->az = (int16_t)((bytes[4] << 8) | bytes[5]);
    data->gx = (int16_t)((bytes[8] << 8) | bytes[9]);
    data->gy = (int16_t)((bytes[10] << 8) | bytes[11]);
    data->gz = (int16_t)((bytes[12] << 8) | bytes[13]);
    return ESP_OK;
}

esp_err_t mpu6050_enable_wom(i2c_port_t port, uint8_t threshold) {
    esp_err_t ret = mpu6050_write_byte(port, MPU6050_ACCEL_CONFIG, 0x01);
    if (ret != ESP_OK) return ret;
    ret = mpu6050_write_byte(port, MPU6050_MOT_THR, threshold);
    if (ret != ESP_OK) return ret;
    ret = mpu6050_write_byte(port, MPU6050_MOT_DUR, 0x01);
    if (ret != ESP_OK) return ret;
    ret = mpu6050_write_byte(port, MPU6050_INT_PIN_CFG, 0x20);
    if (ret != ESP_OK) return ret;
    ret = mpu6050_write_byte(port, MPU6050_INT_ENABLE, 0x40);
    if (ret != ESP_OK) return ret;
    ret = mpu6050_write_byte(port, MPU6050_PWR_MGMT_1_REG, 0x28);
    if (ret != ESP_OK) return ret;
    return mpu6050_write_byte(port, MPU6050_PWR_MGMT_2_REG, 0x07);
}

esp_err_t mpu6050_clear_int(i2c_port_t port) {
    uint8_t status;
    return mpu6050_read_bytes(port, MPU6050_INT_STATUS, &status, 1);
}

