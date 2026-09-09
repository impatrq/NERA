#include "mock_temperature_sensor.h"

#include "core/event_bus.h"
#include "utils/logger.h"

static const char *TAG = NERA_TAG_SENSOR;

esp_err_t MockTemperatureSensor::begin()
{
    temperature_celsius_ = 36.5f;
    direction_ = 1;
    valid_ = true;
    NERA_LOGI(TAG, "Mock temperature sensor initialized");
    return ESP_OK;
}

esp_err_t MockTemperatureSensor::update()
{
    if (!valid_) {
        return ESP_ERR_INVALID_STATE;
    }

    temperature_celsius_ += static_cast<float>(direction_) * 0.1f;
    if (temperature_celsius_ >= 37.2f) {
        direction_ = -1;
    } else if (temperature_celsius_ <= 36.0f) {
        direction_ = 1;
    }

    NeraEventData data = {};
    data.float_val = temperature_celsius_;
    return event_bus_publish(NERA_EVENT_TEMPERATURE_UPDATED, &data);
}

float MockTemperatureSensor::get_temperature_celsius() const
{
    return temperature_celsius_;
}

bool MockTemperatureSensor::is_valid() const
{
    return valid_;
}
