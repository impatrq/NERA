/** Resumen de salud: filas legibles y estado de adquisicion, sin diagnosticos.
 * HealthService aporta tambien sueno y edad de la ultima lectura. */
#include "ui/screens/watch_screens.h"
#include "ui/theme/nera_theme.h"
#include "esp_timer.h"
#include <stdio.h>
using namespace nera_theme;

void health_screen_create(WatchView &v) {
    const int inner = lv_obj_get_width(v.root) - 2 * margin;
    const char *names[] = {"Pulso", "Temp.", "Sueno"};
    const WatchPage pages[] = {WatchPage::Heart, WatchPage::Temperature, WatchPage::Sleep};
    for (int i = 0; i < 3; ++i) {
        auto *row = metric_row(v.root, 52 + i * 57, inner, 52, names[i], &v.labels[i]);
        watch_navigate_on_tap(row, pages[i]);
    }
    // MODIFICABLE: lectura simple del MPU; no es una medida de sueno.
    label(v.root, margin, 228, 90, &lv_font_montserrat_14, muted, "Movimiento");
    v.labels[4] = label(v.root, 106, 228, 118, &lv_font_montserrat_14, text);
    lv_label_set_long_mode(v.labels[4], LV_LABEL_LONG_DOT);
    v.labels[3] = label(v.root, margin, 250, inner, &lv_font_montserrat_14, muted);
    lv_label_set_long_mode(v.labels[3], LV_LABEL_LONG_DOT);
}
void health_screen_update(WatchView &v, const NeraHealthSnapshot &d) {
    char value[96];
    if (d.heart.is_valid) snprintf(value, sizeof(value), "%.0f BPM", d.heart.bpm);
    else snprintf(value, sizeof(value), "-- BPM");
    set(v.labels[0], value);
    if (d.temperature.is_valid) snprintf(value, sizeof(value), "%.1f %s", d.temperature.celsius, degrees);
    else snprintf(value, sizeof(value), "-- %s", degrees);
    set(v.labels[1], value);
    if (d.uses_mock_data) snprintf(value, sizeof(value), "%uh %02um", (d.sleep.light_minutes + d.sleep.deep_minutes) / 60,
                                 (d.sleep.light_minutes + d.sleep.deep_minutes) % 60);
    else snprintf(value, sizeof(value), "--");
    set(v.labels[2], value);
    // FUNCIONAL: mostrar solo lecturas recientes; no conservar un estado viejo
    // si el MPU se desconecta mientras Salud permanece abierta.
    const NeraMotionData &motion = d.state.motion;
    const bool motion_current = motion.sensor_ok && motion.is_valid &&
        motion.last_update_us > 0 &&
        esp_timer_get_time() - motion.last_update_us <= 3000000;
    set(v.labels[4], motion_current ? (motion.is_moving ? "En movimiento" : "Sin movimiento")
                                    : "Sin lectura");
    const int64_t last = d.heart.last_update_us > d.temperature.last_update_us ?
                         d.heart.last_update_us : d.temperature.last_update_us;
    if (last > 0) snprintf(value, sizeof(value), "%s / hace %lld s",
                          d.uses_mock_data ? "Demo" : "Sensor",
                          (long long)((esp_timer_get_time() - last) / 1000000));
    else snprintf(value, sizeof(value), "Sensores no disponibles");
    set(v.labels[3], value);
}
