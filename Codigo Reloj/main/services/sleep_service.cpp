#include "services/sleep_service.h"

#include "utils/logger.h"

static NeraSleepData s_sleep_data = {
    .duration_minutes = 432,
    .quality_percent = 82,
    .light_minutes = 246,
    .deep_minutes = 138,
    .awake_minutes = 48,
    .is_estimate = true,
};

esp_err_t sleep_service_init(void)
{
    NERA_LOGI(NERA_TAG_SLEEP, "SleepService inicializado en modo estimacion");
    return ESP_OK;
}

esp_err_t sleep_service_get_data(NeraSleepData *data)
{
    if (data == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    *data = s_sleep_data;
    return ESP_OK;
}
