#pragma once

#include "core/app_state.h"
#include "esp_err.h"

typedef struct {
    NeraHeartData heart;
    NeraTempData temperature;
    NeraBatteryData battery;
    bool overall_ok;
    bool uses_mock_data;
} NeraHealthSnapshot;

// HealthService ofrece una lectura coherente para la UI sin acoplarla a cada sensor.
esp_err_t health_service_get_snapshot(NeraHealthSnapshot *snapshot);
