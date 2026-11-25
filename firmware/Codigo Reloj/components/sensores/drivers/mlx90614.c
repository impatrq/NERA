// FUNCIONAL: implementacion trasladada sin cambiar las transacciones I2C.
#include "mlx90614.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MPUMLX";
#define I2C_TIMEOUT_TICKS pdMS_TO_TICKS(100)

static uint8_t crc8(uint8_t crc, uint8_t byte) {
    crc ^= byte;
    for (unsigned bit = 0; bit < 8; ++bit)
        crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
    return crc;
}

static esp_err_t mlx90614_read_temperature(i2c_port_t port, uint8_t reg, float *celsius) {
    uint8_t bytes[3];
    esp_err_t ret = i2c_master_write_read_device(port, MLX90614_I2C_ADDR,
                                                  &reg, 1, bytes, sizeof(bytes), I2C_TIMEOUT_TICKS);
    if (ret != ESP_OK) return ret;
    // PEC SMBus: direccion de escritura, comando, direccion de lectura y datos.
    uint8_t pec = 0;
    pec = crc8(pec, MLX90614_I2C_ADDR << 1);
    pec = crc8(pec, reg);
    pec = crc8(pec, (MLX90614_I2C_ADDR << 1) | 1);
    pec = crc8(pec, bytes[0]);
    pec = crc8(pec, bytes[1]);
    if (pec != bytes[2]) return ESP_ERR_INVALID_CRC;
    const uint16_t raw = (uint16_t)((bytes[1] << 8) | bytes[0]);
    if ((raw & 0x8000) || raw == 0) return ESP_ERR_INVALID_RESPONSE;
    *celsius = raw * 0.02f - 273.15f;
    return ESP_OK;
}

esp_err_t mlx90614_read(mlx90614_data_t *data, i2c_port_t port) {
    if (!data) return ESP_ERR_INVALID_ARG;
    mlx90614_data_t sample;
    esp_err_t ret = mlx90614_read_temperature(port, 0x07, &sample.mlx_object_temp);
    if (ret != ESP_OK) return ret;
    ret = mlx90614_read_temperature(port, 0x06, &sample.mlx_amb_temp);
    if (ret == ESP_OK) *data = sample; // Publicar las dos temperaturas juntas.
    return ret;
}
