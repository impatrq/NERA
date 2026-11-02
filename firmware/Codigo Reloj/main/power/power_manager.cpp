/** Politica de backlight: inicializar tras LEDC. Luego solo LVGL modifica
 * brillo y ahorro. La inactividad atenua sin detener CPU ni perder el touch. */
#include "power/power_manager.h"
#include "display/display_manager.h"
#include "storage/storage_manager.h"
#include "utils/logger.h"
static bool saver = false, dimmed = false;
static uint8_t normal = NERA_BL_DEFAULT;
static esp_err_t apply(uint8_t value, bool saving, bool dim) {
    const uint8_t target = (saving || dim) && value > NERA_BL_SAVER ? NERA_BL_SAVER : value;
    if (target == display_manager_get_brightness()) return ESP_OK;
    return display_manager_set_brightness(target);
}
esp_err_t power_manager_init(void) {
    NeraUserSettings s = {};
    esp_err_t err = storage_manager_load_settings(&s);
    if (err != ESP_OK) NERA_LOGW(NERA_TAG_POWER, "Preferencias: %s", esp_err_to_name(err));
    normal = s.brightness < NERA_BL_MIN ? NERA_BL_MIN : s.brightness;
    saver = s.power_saver_enabled;
    return apply(normal, saver, false);
}
esp_err_t power_manager_set_saver(bool enabled) {
    esp_err_t err = apply(normal, enabled, dimmed);
    if (err == ESP_OK) saver = enabled;
    return err;
}
esp_err_t power_manager_set_brightness(uint8_t value) {
    if (value < NERA_BL_MIN) value = NERA_BL_MIN;
    esp_err_t err = apply(value, saver, dimmed);
    if (err == ESP_OK) normal = value;
    return err;
}
bool power_manager_is_saver_enabled(void) { return saver; }
uint8_t power_manager_get_brightness(void) { return normal; }
void power_manager_tick(uint32_t inactive_ms) {
    bool dim = inactive_ms >= NERA_SCREEN_DIM_TIMEOUT_MS;
    if (dim != dimmed && apply(normal, saver, dim) == ESP_OK) dimmed = dim;
}
