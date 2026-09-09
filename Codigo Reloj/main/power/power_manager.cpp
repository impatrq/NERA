#include "power/power_manager.h"

#include "display/display_manager.h"
#include "nera_config.h"
#include "storage/storage_manager.h"
#include "utils/logger.h"

static bool s_saver_enabled = false;
static uint8_t s_normal_brightness = NERA_BL_DEFAULT;

esp_err_t power_manager_init(void)
{
    NeraUserSettings settings = {};
    storage_manager_load_settings(&settings);
    s_normal_brightness = settings.brightness;
    display_manager_set_brightness(s_normal_brightness);
    s_saver_enabled = settings.power_saver_enabled;
    if (s_saver_enabled) {
        display_manager_set_brightness(NERA_BL_SAVER);
    }
    NERA_LOGI(NERA_TAG_POWER, "PowerManager listo; brillo: %u", s_normal_brightness);
    return ESP_OK;
}

esp_err_t power_manager_set_saver(bool enabled)
{
    s_saver_enabled = enabled;
    const uint8_t target = enabled ? NERA_BL_SAVER : s_normal_brightness;
    esp_err_t err = display_manager_fade_brightness(target, NERA_UI_TRANSITION_MS);
    if (err == ESP_OK) {
        NERA_LOGI(NERA_TAG_POWER, "Ahorro de energia: %s", enabled ? "ACTIVO" : "INACTIVO");
    }
    return err;
}

bool power_manager_is_saver_enabled(void)
{
    return s_saver_enabled;
}

uint8_t power_manager_get_brightness(void)
{
    return display_manager_get_brightness();
}
