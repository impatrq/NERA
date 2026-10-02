#pragma once

#include "temperature_sensor.h"

class MockTemperatureSensor final : public TemperatureSensor {
public:
    esp_err_t begin() override;
    esp_err_t update() override;
    float get_temperature_celsius() const override;
    bool is_valid() const override;

private:
    float temperature_celsius_ = 36.5f;
    int8_t direction_ = 1;
    bool valid_ = false;
};
