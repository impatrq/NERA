#pragma once

#include "core/app_state.h"
#include "esp_err.h"

class MockBatteryManager {
public:
    esp_err_t begin();
    esp_err_t update();
    NeraBatteryData get_data() const;

private:
    uint8_t percentage_ = 100;
    NeraBatteryData data_ = {};
};
