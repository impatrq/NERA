/** Ajustes tactiles: PWM inmediato y guardado mediante cola de StorageManager.
 * BLE, vibracion y avisos se muestran deshabilitados hasta tener soporte real. */
#include "ui/screens/watch_screens.h"
#include "ui/theme/nera_theme.h"
#include "power/power_manager.h"
#include "storage/storage_manager.h"
#include <stdio.h>
using namespace nera_theme;
static NeraUserSettings preferences;
static esp_err_t last_error = ESP_OK;

static void change(lv_event_t *event) {
    auto *view = static_cast<WatchView *>(lv_event_get_user_data(event));
    auto *target = lv_event_get_target(event);
    if (target == view->controls[0]) {
        const uint8_t value = lv_slider_get_value(target);
        last_error = power_manager_set_brightness(value);
        if (last_error == ESP_OK) preferences.brightness = value;
        if (lv_event_get_code(event) != LV_EVENT_RELEASED) return;
    } else {
        const bool enabled = lv_obj_has_state(target, LV_STATE_CHECKED);
        last_error = power_manager_set_saver(enabled);
        if (last_error == ESP_OK) preferences.power_saver_enabled = enabled;
        else if (power_manager_is_saver_enabled()) lv_obj_add_state(target, LV_STATE_CHECKED);
        else lv_obj_clear_state(target, LV_STATE_CHECKED);
    }
    if (last_error == ESP_OK) last_error = storage_manager_request_settings(&preferences);
}
void settings_screen_create(WatchView &v, bool quick) {
    last_error = storage_manager_load_settings(&preferences);
    preferences.brightness = power_manager_get_brightness();
    preferences.power_saver_enabled = power_manager_is_saver_enabled();
    const int w = lv_obj_get_width(v.root), inner = w - 2 * margin;
    auto *content = lv_obj_create(v.root);
    lv_obj_remove_style_all(content);
    lv_obj_set_pos(content, 0, 48);
    lv_obj_set_size(content, w, lv_obj_get_height(v.root) - 96);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);
    if (quick) lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    label(content, margin, 4, inner - 60, &lv_font_montserrat_20, text, "Brillo");
    v.labels[0] = label(content, w - 76, 4, 60, &lv_font_montserrat_20, text);
    auto *slider = lv_slider_create(content);
    v.controls[0] = slider;
    lv_obj_set_pos(slider, margin + 8, 44);
    lv_obj_set_size(slider, inner - 16, 16);
    lv_obj_set_ext_click_area(slider, 14);
    lv_slider_set_range(slider, NERA_BL_MIN, 255);
    lv_slider_set_value(slider, preferences.brightness, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(accent), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(text), LV_PART_KNOB);
    lv_obj_clear_flag(slider, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_event_cb(slider, change, LV_EVENT_VALUE_CHANGED, &v);
    lv_obj_add_event_cb(slider, change, LV_EVENT_RELEASED, &v);
    const char *names[] = {"Ahorro", "Bluetooth", "Vibracion", "Avisos"};
    for (int i = 0; i < (quick ? 1 : 4); ++i) {
        label(content, margin, 98 + i * 56, inner - 58, &lv_font_montserrat_20,
              i == 0 ? text : muted, names[i]);
        auto *toggle = lv_switch_create(content);
        lv_obj_set_pos(toggle, w - margin - 44, 94 + i * 56);
        lv_obj_set_size(toggle, 44, 26);
        lv_obj_set_ext_click_area(toggle, 9);
        lv_obj_set_style_bg_color(toggle, lv_color_hex(accent), LV_PART_INDICATOR | LV_STATE_CHECKED);
        if (i == 0) {
            v.controls[1] = toggle;
            if (preferences.power_saver_enabled) lv_obj_add_state(toggle, LV_STATE_CHECKED);
            lv_obj_add_event_cb(toggle, change, LV_EVENT_VALUE_CHANGED, &v);
        } else lv_obj_add_state(toggle, LV_STATE_DISABLED);
    }
    if (quick) {
        auto *settings = panel(content, margin, 146, inner, 48);
        label(settings, 12, 12, inner - 24, &lv_font_montserrat_20, text, LV_SYMBOL_SETTINGS " Ajustes");
        watch_navigate_on_tap(settings, WatchPage::Settings);
    } else label(content, margin, 310, inner, &lv_font_montserrat_14, muted,
          "NERA " NERA_FW_VERSION_STR "\nESP32-S3 / LCD 240x320\nBLE, avisos y vibracion\nno disponibles");
    v.labels[1] = label(content, margin, quick ? 204 : 390, inner, &lv_font_montserrat_14, muted, "");
}
void settings_screen_update(WatchView &v) {
    // Panel rapido y ajustes comparten el estado, pero conservan sus propios widgets.
    // No mover el knob durante un arrastre. Al entrar, refleja cambios de la otra vista.
    if (!lv_slider_is_dragged(v.controls[0])) {
        lv_slider_set_value(v.controls[0], power_manager_get_brightness(), LV_ANIM_OFF);
        if (power_manager_is_saver_enabled()) lv_obj_add_state(v.controls[1], LV_STATE_CHECKED);
        else lv_obj_clear_state(v.controls[1], LV_STATE_CHECKED);
    }
    char value[20];
    snprintf(value, sizeof(value), "%u%%", (unsigned)(power_manager_get_brightness() * 100 / 255));
    set(v.labels[0], value);
    const esp_err_t status = storage_manager_settings_status();
    set(v.labels[1], last_error != ESP_OK ? "No se pudo aplicar" :
        status == ESP_ERR_NOT_FINISHED ? "Guardando..." : status == ESP_OK ? "Ajustes guardados" : "Error al guardar");
}
