/** Sueno demo: separa tiempo dormido de tiempo despierto.
 * Las barras representan duraciones del servicio, no fases medidas con IMU. */
#include "ui/screens/watch_screens.h"
#include "ui/theme/nera_theme.h"
#include <stdio.h>
using namespace nera_theme;
void sleep_screen_create(WatchView &v) {
    const int inner = lv_obj_get_width(v.root) - 2 * margin;
    v.labels[0] = label(v.root, margin, 48, inner, &lv_font_montserrat_48, text);
    lv_label_set_long_mode(v.labels[0], LV_LABEL_LONG_DOT);
    v.labels[1] = label(v.root, margin, 107, inner, &lv_font_montserrat_14, muted);
    const char *names[] = {"Ligero", "Profundo", "Despierto"};
    const uint32_t colors[] = {accent, accent, muted};
    for (int i = 0; i < 3; ++i) {
        label(v.root, margin, 136 + 39 * i, 94, &lv_font_montserrat_14, muted, names[i]);
        v.labels[2 + i] = label(v.root, 115, 131 + 39 * i, inner - 99, &lv_font_montserrat_20, text);
        auto *bar = lv_bar_create(v.root);
        lv_obj_set_pos(bar, margin, 159 + i * 39);
        lv_obj_set_size(bar, inner, 4);
        lv_obj_set_style_bg_color(bar, lv_color_hex(line), 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(colors[i]), LV_PART_INDICATOR);
        lv_bar_set_range(bar, 0, 600);
        v.controls[i] = bar;
    }
    v.labels[5] = label(v.root, margin, 252, inner, &lv_font_montserrat_14, muted);
}
void sleep_screen_update(WatchView &v, const NeraHealthSnapshot &d) {
    char value[64];
    const unsigned total = d.sleep.light_minutes + d.sleep.deep_minutes;
    if (d.uses_mock_data) snprintf(value, sizeof(value), "%uh %02um", total / 60, total % 60);
    else snprintf(value, sizeof(value), "--");
    set(v.labels[0], value);
    if (d.uses_mock_data) snprintf(value, sizeof(value), "Calidad demo: %u%%", d.sleep.quality_percent);
    else snprintf(value, sizeof(value), "Sin registro de sueno");
    set(v.labels[1], value);
    set(v.labels[5], d.uses_mock_data ? "Datos de demostracion" : "Sensor no disponible");
    const unsigned durations[] = {d.sleep.light_minutes, d.sleep.deep_minutes, d.sleep.awake_minutes};
    for (int i = 0; i < 3; ++i) {
        if (d.uses_mock_data) snprintf(value, sizeof(value), "%uh %02um", durations[i] / 60, durations[i] % 60);
        else snprintf(value, sizeof(value), "--");
        set(v.labels[i + 2], value);
        if (lv_bar_get_value(v.controls[i]) != (int)durations[i]) lv_bar_set_value(v.controls[i], durations[i], LV_ANIM_OFF);
    }
}
