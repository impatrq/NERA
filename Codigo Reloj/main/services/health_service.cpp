#include "services/health_service.h"

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

    snapshot->heart = state.heart;
    snapshot->temperature = state.temp;
    snapshot->battery = state.battery;
    snapshot->overall_ok = state.heart.is_valid && state.temp.is_valid &&
                           state.battery.sensor_ok;
    snapshot->uses_mock_data = state.use_mocks;
    return ESP_OK;
}
