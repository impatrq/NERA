/** Snapshot unico para las pantallas. Calcula estadisticas del historial
 * protegido por AppState; nunca accede a drivers desde la tarea grafica. */
#include "services/health_service.h"
#include <math.h>

#include "utils/logger.h"

esp_err_t health_service_get_snapshot(NeraHealthSnapshot *snapshot)
{
    if (snapshot == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    NeraAppState state = {};
    esp_err_t err = app_state_get(&state);
    if (err != ESP_OK) {
        return err;
    }

    *snapshot = {};
    snapshot->state = state;
    sleep_service_get_data(&snapshot->sleep);
    unsigned valid = 0;
    for (unsigned i = 0; i < state.heart_history_count; ++i) {
        const float value = state.heart_history[i];
        if (!isfinite(value) || value <= 0) continue;
        if (valid == 0 || value < snapshot->minimum) snapshot->minimum = value;
        if (valid == 0 || value > snapshot->maximum) snapshot->maximum = value;
        snapshot->average += value;
        ++valid;
    }
    if (valid) snapshot->average /= valid;
    snapshot->has_trend = state.temp_history_count >= 2;
    if (snapshot->has_trend) {
        snapshot->temperature_delta = state.temp_history[state.temp_history_count - 1] -
                                       state.temp_history[state.temp_history_count - 2];
        snapshot->has_trend = isfinite(snapshot->temperature_delta);
    }
    snapshot->heart = state.heart;
    snapshot->temperature = state.temp;
    snapshot->battery = state.battery;
    snapshot->overall_ok = state.heart.is_valid && state.temp.is_valid &&
                           state.battery.sensor_ok;
    snapshot->uses_mock_data = state.use_mocks;
    return ESP_OK;
}
