/** Inicio: hora dominante y tres accesos tactiles. HealthService aporta los datos;
 * UIManager decide el destino del toque sin acoplar esta vista a los drivers. */
#include "ui/screens/watch_screens.h"
#include "ui/theme/nera_theme.h"
#include <stdio.h>
using namespace nera_theme;

void watchface_create(WatchView &v) {
    const int w = lv_obj_get_width(v.root), inner = w - 2 * margin;
    label(v.root, margin, 12, 60, &lv_font_montserrat_14, accent, "NERA");
    v.labels[7] = label(v.root, 77, 12, 16, &lv_font_montserrat_14, muted, LV_SYMBOL_BLUETOOTH);
    v.labels[5] = label(v.root, 97, 12, 60, &lv_font_montserrat_14, muted);
    v.labels[0] = label(v.root, w - 76, 12, 60, &lv_font_montserrat_14, muted);
    lv_obj_set_style_text_align(v.labels[0], LV_TEXT_ALIGN_RIGHT, 0);
    v.labels[1] = label(v.root, margin, 34, inner, &lv_font_montserrat_48, text, "--:--");
    lv_obj_set_style_text_align(v.labels[1], LV_TEXT_ALIGN_CENTER, 0);
    v.labels[2] = label(v.root, margin, 94, inner, &lv_font_montserrat_14, muted);
    lv_obj_set_style_text_align(v.labels[2], LV_TEXT_ALIGN_CENTER, 0);
    watch_navigate_on_tap(metric_row(v.root, 116, inner, 46, "Pulso", &v.labels[3]), WatchPage::Heart);
    watch_navigate_on_tap(metric_row(v.root, 168, inner, 46, "Temp.", &v.labels[4]), WatchPage::Temperature);
    watch_navigate_on_tap(metric_row(v.root, 220, inner, 46, "Sueno", &v.labels[6]), WatchPage::Sleep);
}

void watchface_update(WatchView &v, const NeraHealthSnapshot &d) {
    char value[80];
    const char *battery_icon = d.battery.state == NERA_BATTERY_CHARGING ? LV_SYMBOL_CHARGE :
        d.battery.percentage > 80 ? LV_SYMBOL_BATTERY_FULL :
        d.battery.percentage > 30 ? LV_SYMBOL_BATTERY_2 : LV_SYMBOL_BATTERY_EMPTY;
    if (d.battery.sensor_ok) snprintf(value, sizeof(value), "%s %u%%", battery_icon, d.battery.percentage);
    else snprintf(value, sizeof(value), LV_SYMBOL_BATTERY_EMPTY " --");
    set(v.labels[0], value);
    if (d.state.datetime.synced) {
        snprintf(value, sizeof(value), "%02u:%02u", d.state.datetime.hour, d.state.datetime.minute);
        set(v.labels[1], value);
        snprintf(value, sizeof(value), "%02u / %02u / %u", d.state.datetime.day,
                 d.state.datetime.month, d.state.datetime.year);
    } else {
        set(v.labels[1], "--:--");
        snprintf(value, sizeof(value), "Hora sin sincronizar");
    }
    set(v.labels[2], value);
    if (d.heart.is_valid) snprintf(value, sizeof(value), "%.0f BPM", d.heart.bpm);
    else snprintf(value, sizeof(value), "-- BPM");
    set(v.labels[3], value);
    if (d.temperature.is_valid) snprintf(value, sizeof(value), "%.1f %s", d.temperature.celsius, degrees);
    else snprintf(value, sizeof(value), "-- %s", degrees);
    set(v.labels[4], value);
    const unsigned slept = d.sleep.light_minutes + d.sleep.deep_minutes;
    if (d.uses_mock_data) snprintf(value, sizeof(value), "%uh %02um", slept / 60, slept % 60);
    else snprintf(value, sizeof(value), "--");
    set(v.labels[6], value);
    const auto connection_color = lv_color_hex(d.state.ble_state == NERA_BLE_CONNECTED ? accent : muted);
    if (lv_color_to32(lv_obj_get_style_text_color(v.labels[7], 0)) != lv_color_to32(connection_color))
        lv_obj_set_style_text_color(v.labels[7], connection_color, 0);
    set(v.labels[5], d.uses_mock_data ? "Demo" : d.overall_ok ? "OK" : "--");
}
