#pragma once

#include "esp_err.h"

typedef struct {
    uint8_t brightness;
    bool notifications_enabled;
    bool vibration_enabled;
    bool power_saver_enabled;
} NeraUserSettings;

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t storage_manager_init(void);
void storage_manager_task(void *arg);
esp_err_t storage_manager_load_settings(NeraUserSettings *settings);
esp_err_t storage_manager_save_settings(const NeraUserSettings *settings);

#ifdef __cplusplus
}
#endif
