#include "battery_manager.h"

#include "utils/logger.h"

esp_err_t MockBatteryManager::begin()
{
    percentage_ = 100;
    NERA_LOGI(NERA_TAG_BATTERY, "Mock battery manager initialized");
    return ESP_OK;
}

esp_err_t MockBatteryManager::update()
{
    data_ = {};
    data_.percentage = percentage_;
    data_.voltage_mv = 4200.0f - ((100.0f - percentage_) * 12.0f);
    data_.sensor_ok = true;
    data_.state = percentage_ <= 5 ? NERA_BATTERY_CRITICAL :
                  percentage_ <= 20 ? NERA_BATTERY_LOW :
                  percentage_ >= 100 ? NERA_BATTERY_FULL :
                  NERA_BATTERY_DISCHARGING;
    if (percentage_ > 10) {
        percentage_--;
    } else {
        percentage_ = 100;
    }
    return ESP_OK;
}

NeraBatteryData MockBatteryManager::get_data() const
{
    return data_;
}
