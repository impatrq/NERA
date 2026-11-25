// FUNCIONAL: implementacion trasladada sin cambiar las transacciones I2C.
#include "i2c_bus.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MPUMLX";
#define I2C_TIMEOUT_TICKS pdMS_TO_TICKS(100)

// El componente posee I2C1; el CST816D de la pantalla sigue en I2C0.
esp_err_t init_i2c(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = MLX_MPU6050_SDA,
        .scl_io_num = MLX_MPU6050_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    esp_err_t ret = i2c_param_config(MLX_MPU6050_I2C_PORT, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Configuracion I2C1: %s", esp_err_to_name(ret));
        return ret;
    }
    ret = i2c_driver_install(MLX_MPU6050_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
    if (ret == ESP_ERR_INVALID_STATE) return ESP_OK;
    if (ret != ESP_OK) ESP_LOGE(TAG, "Instalacion I2C1: %s", esp_err_to_name(ret));
    else ESP_LOGI(TAG, "I2C1 inicializado: SDA=%d SCL=%d", MLX_MPU6050_SDA, MLX_MPU6050_SCL);
    return ret;
}

