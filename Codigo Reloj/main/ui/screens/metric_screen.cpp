/** Detalles de pulso y temperatura. Grafico LVGL de ventana reciente;
 * solo se invalida cuando cambia la serie, no en cada tick de interfaz. */
#include "ui/screens/watch_screens.h"
#include "ui/theme/nera_theme.h"
#include <math.h>
#include <stdio.h>
using namespace nera_theme;

void metric_screen_create(WatchView &v, bool thermal) {
    const int inner = lv_obj_get_width(v.root) - 2 * margin;
    const uint32_t color = accent;
    v.labels[0] = label(v.root, margin, 52, inner, &lv_font_montserrat_48, text, "--");
    lv_label_set_long_mode(v.labels[0], LV_LABEL_LONG_DOT);
    lv_obj_set_width(v.labels[0], thermal ? 146 : 126);
    lv_obj_set_style_text_align(v.labels[0], LV_TEXT_ALIGN_RIGHT, 0);
    label(v.root, thermal ? 168 : 148, 77, thermal ? 56 : 76,
          &lv_font_montserrat_20, muted, thermal ? degrees : "BPM");
    v.labels[1] = label(v.root, margin, 106, inner, &lv_font_montserrat_14, muted);
    v.chart = lv_chart_create(v.root);
    lv_obj_set_pos(v.chart, margin, 136);
    lv_obj_set_size(v.chart, inner, 82);
    lv_obj_set_style_bg_color(v.chart, lv_color_hex(bg), 0);
    lv_obj_set_style_border_width(v.chart, 0, 0);
    lv_obj_set_style_pad_all(v.chart, 4, 0);
    lv_obj_set_style_line_color(v.chart, lv_color_hex(line), 0);
    lv_obj_set_style_line_width(v.chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_size(v.chart, 0, LV_PART_INDICATOR);
    lv_obj_clear_flag(v.chart, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_chart_set_point_count(v.chart, NERA_HISTORY_BUFFER_SIZE);
    lv_chart_set_div_line_count(v.chart, 3, 0);
    v.series = lv_chart_add_series(v.chart, lv_color_hex(color), LV_CHART_AXIS_PRIMARY_Y);
    v.labels[2] = label(v.root, margin, 226, inner, &lv_font_montserrat_14, text);
    v.labels[3] = label(v.root, margin, 247, inner, &lv_font_montserrat_14, muted);
}

void metric_screen_update(WatchView &v, const NeraHealthSnapshot &d, bool thermal) {
    char value[96];
    if (thermal ? d.temperature.is_valid : d.heart.is_valid)
        snprintf(value, sizeof(value), thermal ? "%.1f" : "%.0f", thermal ? d.temperature.celsius : d.heart.bpm);
    else snprintf(value, sizeof(value), "--");
    set(v.labels[0], value);
    const bool valid = thermal ? d.temperature.is_valid : d.heart.is_valid;
    set(v.labels[1], valid ? (d.uses_mock_data ? "Datos de demostracion" : "Lectura del sensor") :
        "Esperando lectura");
    const auto &state = d.state;
    const float *values = thermal ? state.temp_history : state.heart_history;
    const uint8_t count = thermal ? state.temp_history_count : state.heart_history_count;
    if (thermal) {
        if (d.has_trend) snprintf(value, sizeof(value), "%s  %+.1f %s", d.temperature_delta > 0.05f ? "Subiendo" :
             d.temperature_delta < -0.05f ? "Bajando" : "Estable", d.temperature_delta, degrees);
        else snprintf(value, sizeof(value), "Tendencia sin datos");
    } else if (count) snprintf(value, sizeof(value), "Min %.0f   Med %.0f   Max %.0f", d.minimum, d.average, d.maximum);
    else snprintf(value, sizeof(value), "Historial sin datos");
    set(v.labels[2], value);
    snprintf(value, sizeof(value), thermal ? "%u muestras / no clinico" : "%u muestras recientes", count);
    set(v.labels[3], value);
    if (count == v.last_count && memcmp(values, v.last_values, count * sizeof(float)) == 0) return;
    float low = thermal ? 350.f : 50.f, high = thermal ? 380.f : 110.f;
    for (unsigned i = 0; i < NERA_HISTORY_BUFFER_SIZE; ++i) {
        lv_coord_t point = LV_CHART_POINT_NONE;
        if (i < count && isfinite(values[i])) {
            float sample = values[i] * (thermal ? 10 : 1);
            sample = fmaxf(-30000, fminf(30000, sample));
            low = fminf(low, sample - 5); high = fmaxf(high, sample + 5);
            point = (lv_coord_t)sample;
        }
        lv_chart_set_value_by_id(v.chart, v.series, i, point);
    }
    lv_chart_set_range(v.chart, LV_CHART_AXIS_PRIMARY_Y, (lv_coord_t)low, (lv_coord_t)high);
    memcpy(v.last_values, values, count * sizeof(float));
    v.last_count = count;
}
