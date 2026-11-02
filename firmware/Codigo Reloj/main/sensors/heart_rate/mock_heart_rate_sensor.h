#pragma once

#include "heart_rate_sensor.h"

class MockHeartRateSensor final : public HeartRateSensor {
public:
    esp_err_t begin() override;
    esp_err_t update() override;
    float get_bpm() const override;
    bool is_valid() const override;

private:
    float bpm_ = 72.0f;
    int8_t direction_ = 1;
    bool valid_ = false;
};
