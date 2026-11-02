/** Contrato entre servicios y UI. Un snapshot contiene una copia consistente
 * del estado y las estadisticas; las vistas no conocen los drivers. */
#pragma once

#include "core/app_state.h"
#include "services/sleep_service.h"
#include "esp_err.h"

typedef struct {
    NeraHeartData heart;
    NeraTempData temperature;
    NeraBatteryData battery;
    bool overall_ok;
    bool uses_mock_data;
    NeraAppState state;
    NeraSleepData sleep;
    float minimum;
    float maximum;
    float average;
    float temperature_delta;
    bool has_trend;
} NeraHealthSnapshot;

// HealthService ofrece una lectura coherente para la UI sin acoplarla a cada sensor.
esp_err_t health_service_get_snapshot(NeraHealthSnapshot *snapshot);
