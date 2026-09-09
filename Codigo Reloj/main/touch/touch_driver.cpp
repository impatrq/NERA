#include "touch_driver.h"

#include "driver/i2c.h"
#include "nera_config.h"
#include "utils/logger.h"

static const char *TAG = NERA_TAG_TOUCH;
static bool s_initialized = false;

esp_err_t touch_driver_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    i2c_config_t config = {};
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = (gpio_num_t)NERA_TOUCH_PIN_SDA;
    config.scl_io_num = (gpio_num_t)NERA_TOUCH_PIN_SCL;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = NERA_I2C_FREQ_HZ;

    esp_err_t ret = i2c_param_config(NERA_I2C_PORT, &config);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        return ret;
    }

    ret = i2c_driver_install(NERA_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        NERA_LOGW(TAG, "CST816D no disponible: %s", esp_err_to_name(ret));
        return ret;
    }

    s_initialized = true;
    NERA_LOGI(TAG, "CST816D touch inicializado en I2C");
    return ESP_OK;
}

void touch_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    data->state = LV_INDEV_STATE_RELEASED;

    if (!s_initialized) {
        return;
    }

    uint8_t registers[5] = {};
    const uint8_t register_address = 0x02;
    esp_err_t ret = i2c_master_write_read_device(
        NERA_I2C_PORT,
        NERA_TOUCH_I2C_ADDR,
        &register_address,
        1,
        registers,
        sizeof(registers),
        pdMS_TO_TICKS(20));
    if (ret != ESP_OK || (registers[0] & 0x0F) == 0) {
        return;
    }

    data->point.x = (lv_coord_t)(((registers[1] & 0x0F) << 8) | registers[2]);
    data->point.y = (lv_coord_t)(((registers[3] & 0x0F) << 8) | registers[4]);
    data->state = LV_INDEV_STATE_PRESSED;
}
