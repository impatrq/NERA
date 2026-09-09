#pragma once

#include "esp_err.h"

class HeartRateSensor {
public:
    virtual ~HeartRateSensor() = default;

    virtual esp_err_t begin() = 0;
    virtual esp_err_t update() = 0;
    virtual float get_bpm() const = 0;
    virtual bool is_valid() const = 0;
};
