#include "mock_heart_rate_sensor.h"

#include "core/event_bus.h"
#include "utils/logger.h"

static const char *TAG = NERA_TAG_SENSOR;

esp_err_t MockHeartRateSensor::begin()
{
    bpm_ = 72.0f;
    direction_ = 1;
    valid_ = true;
    NERA_LOGI(TAG, "Mock heart-rate sensor initialized");
    return ESP_OK;
}

esp_err_t MockHeartRateSensor::update()
{
    if (!valid_) {
        return ESP_ERR_INVALID_STATE;
    }

    bpm_ += static_cast<float>(direction_);
    if (bpm_ >= 78.0f) {
        direction_ = -1;
    } else if (bpm_ <= 68.0f) {
        direction_ = 1;
    }

    NeraEventData data = {};
    data.float_val = bpm_;
    return event_bus_publish(NERA_EVENT_HEART_RATE_UPDATED, &data);
}

float MockHeartRateSensor::get_bpm() const
{
    return bpm_;
}

bool MockHeartRateSensor::is_valid() const
{
    return valid_;
}
