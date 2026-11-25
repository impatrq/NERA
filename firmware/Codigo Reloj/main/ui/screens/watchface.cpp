/** Inicio NERA: composicion tipografica sin tarjetas permanentes.
 * HealthService entrega datos; UIManager conserva toda la navegacion. */
#include "ui/screens/watch_screens.h"
#include "ui/theme/nera_theme.h"
#include <stdio.h>
using namespace nera_theme;

namespace {
// 1. CONFIGURACION VISUAL ----------------------------------------------
// MODIFICABLE: posiciones en pixeles para 240x320. Margen y paleta vienen
// de nera_theme.h; no cambiar el tema compartido para ajustar solo Inicio.
constexpr int HEADER_Y = 14;
constexpr int TIME_Y = 54;
constexpr int DATE_Y = 116;
constexpr int PULSE_Y = 153;
constexpr int PULSE_WIDTH = 128;
constexpr int PULSE_HEIGHT = 60;
constexpr int PULSE_UNIT_WIDTH = 34;
constexpr int PULSE_UNIT_GAP = 6;
constexpr int PULSE_UNIT_Y = 34;
constexpr int METRICS_Y = 225;
constexpr int METRIC_GAP = 12;
constexpr int METRIC_HEIGHT = 60;
constexpr int VALUE_Y = 23;
constexpr int FOOTER_BOTTOM = 24;
constexpr int BATTERY_WIDTH = 78;
constexpr int STATUS_WIDTH = 52;
constexpr int BLE_WIDTH = 16;
constexpr int PRESS_RADIUS = 8;
constexpr lv_opa_t PRESS_OPACITY = LV_OPA_30;

// MODIFICABLE: seleccionar fuentes YA compiladas, no un tamano numerico libre.
const lv_font_t *const TIME_FONT = &lv_font_montserrat_48;
const lv_font_t *const VALUE_FONT = &lv_font_montserrat_28;
const lv_font_t *const SECONDARY_FONT = &lv_font_montserrat_20;
const lv_font_t *const SMALL_FONT = &lv_font_montserrat_14;
constexpr uint32_t TIME_COLOR = text;
constexpr uint32_t PULSE_COLOR = heart;
constexpr uint32_t TEMP_COLOR = temp;
constexpr uint32_t SLEEP_COLOR = sleep;
constexpr const char *BRAND_TEXT = "NERA";
constexpr const char *PULSE_TEXT = "Pulso";
constexpr const char *TEMP_TEXT = "Temp. " "\xC2\xB0" "C";
// IMPORTANTE: la fuente disponible no contiene el glifo de la enie.
constexpr const char *SLEEP_TEXT = "Sueno";
constexpr const char *NO_READING = "Sin lectura";
constexpr const char *NO_SLEEP = "Sin registro";

// FUNCIONAL: indices de WatchView, no posiciones visuales. Mantener su
// correspondencia entre creacion y actualizacion de datos.
enum Label { Battery, Time, Date, Pulse, Temperature, Demo, Sleep, Bluetooth,
             PulseCaption, TempCaption, SleepCaption };

// 2. INTERACCION TACTIL ------------------------------------------------
lv_obj_t *metric_target(lv_obj_t *root, int x, int y, int width, int height,
                        WatchPage destination) {
    auto *target = lv_obj_create(root);
    lv_obj_remove_style_all(target);
    lv_obj_set_pos(target, x, y);
    lv_obj_set_size(target, width, height);
    lv_obj_clear_flag(target, LV_OBJ_FLAG_SCROLLABLE);
    // FUNCIONAL: conserva callbacks y proteccion contra taps tras un swipe.
    watch_navigate_on_tap(target, destination);
    // MODIFICABLE: fondo tenue solamente al presionar, no una tarjeta.
    lv_obj_set_style_bg_opa(target, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(target, lv_color_hex(surface), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(target, PRESS_OPACITY, LV_STATE_PRESSED);
    lv_obj_set_style_radius(target, PRESS_RADIUS, 0);
    return target;
}

// 3. TEXTO Y LONGITUD VARIABLE ----------------------------------------
void set_value(lv_obj_t *obj, const char *value, const lv_font_t *preferred) {
    if (strcmp(lv_label_get_text(obj), value) == 0) return;
    // IMPORTANTE: LVGL no escala una fuente cambiando un entero. Elegimos
    // otra fuente existente si el valor no cabe, sin recortar las unidades.
    const lv_font_t *candidates[] = {preferred, SECONDARY_FONT, SMALL_FONT};
    for (const auto *font : candidates) {
        lv_point_t extent;
        lv_txt_get_size(&extent, value, font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (extent.x <= lv_obj_get_width(obj) || font == SMALL_FONT) {
            lv_obj_set_style_text_font(obj, font, 0);
            break;
        }
    }
    set(obj, value);
}
}

// 4. CREACION: HORA, METRICAS Y ESTADOS --------------------------------
void watchface_create(WatchView &v) {
    const int w = lv_obj_get_width(v.root), inner = w - 2 * margin;
    const int column_width = (inner - METRIC_GAP) / 2;
    const int right_x = margin + column_width + METRIC_GAP;
    const int footer_y = lv_obj_get_height(v.root) - FOOTER_BOTTOM;

    label(v.root, margin, HEADER_Y, STATUS_WIDTH, SMALL_FONT, muted, BRAND_TEXT);
    v.labels[Battery] = label(v.root, w - margin - BATTERY_WIDTH, HEADER_Y,
                              BATTERY_WIDTH, SMALL_FONT, muted);
    lv_obj_set_style_text_align(v.labels[Battery], LV_TEXT_ALIGN_RIGHT, 0);
    v.labels[Time] = label(v.root, margin, TIME_Y, inner, TIME_FONT, TIME_COLOR, "--:--");
    v.labels[Date] = label(v.root, margin, DATE_Y, inner, SMALL_FONT, muted,
                           "Hora sin sincronizar");

    auto *pulse = metric_target(v.root, w - margin - PULSE_WIDTH, PULSE_Y,
                                PULSE_WIDTH, PULSE_HEIGHT, WatchPage::Heart);
    v.labels[PulseCaption] = label(pulse, 0, 0, PULSE_WIDTH, SMALL_FONT, PULSE_COLOR, PULSE_TEXT);
    v.labels[Pulse] = label(pulse, 0, VALUE_Y,
                            PULSE_WIDTH - PULSE_UNIT_WIDTH - PULSE_UNIT_GAP, VALUE_FONT, text);
    label(pulse, PULSE_WIDTH - PULSE_UNIT_WIDTH, PULSE_UNIT_Y,
          PULSE_UNIT_WIDTH, SMALL_FONT, muted, "BPM");
    // Pulso a la derecha: contrapunto a la hora alineada a la izquierda.
    lv_obj_set_style_text_align(v.labels[PulseCaption], LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_align(v.labels[Pulse], LV_TEXT_ALIGN_RIGHT, 0);

    auto *thermal = metric_target(v.root, margin, METRICS_Y, column_width,
                                  METRIC_HEIGHT, WatchPage::Temperature);
    v.labels[TempCaption] = label(thermal, 0, 0, column_width, SMALL_FONT, TEMP_COLOR, TEMP_TEXT);
    v.labels[Temperature] = label(thermal, 0, VALUE_Y, column_width, VALUE_FONT, text);
    auto *sleep_target = metric_target(v.root, right_x, METRICS_Y, column_width,
                                       METRIC_HEIGHT, WatchPage::Sleep);
    v.labels[SleepCaption] = label(sleep_target, 0, 0, column_width, SMALL_FONT, SLEEP_COLOR, SLEEP_TEXT);
    v.labels[Sleep] = label(sleep_target, 0, VALUE_Y, column_width, SECONDARY_FONT, text);

    v.labels[Demo] = label(v.root, margin, footer_y, STATUS_WIDTH, SMALL_FONT, muted);
    v.labels[Bluetooth] = label(v.root, w - margin - BLE_WIDTH, footer_y,
                                BLE_WIDTH, SMALL_FONT, muted, LV_SYMBOL_BLUETOOTH);
    // IMPORTANTE: UIManager crea los puntos del pie y controla transiciones
    // con NERA_UI_TRANSITION_MS. No duplicar indicadores ni agregar timers.
}

// 5. ACTUALIZACION DE DATOS --------------------------------------------
// FUNCIONAL: presenta el snapshot; no lee sensores ni modifica servicios.
// set()/set_value() solo invalidan los textos cuando cambia su contenido.
void watchface_update(WatchView &v, const NeraHealthSnapshot &d) {
    char value[80];
    const char *battery_icon = d.battery.state == NERA_BATTERY_CHARGING ? LV_SYMBOL_CHARGE :
        d.battery.percentage > 80 ? LV_SYMBOL_BATTERY_FULL :
        d.battery.percentage > 30 ? LV_SYMBOL_BATTERY_2 : LV_SYMBOL_BATTERY_EMPTY;
    if (d.battery.sensor_ok) snprintf(value, sizeof(value), "%s %u%%", battery_icon, d.battery.percentage);
    else snprintf(value, sizeof(value), LV_SYMBOL_BATTERY_EMPTY " --");
    set(v.labels[Battery], value);
    if (d.state.datetime.synced) {
        snprintf(value, sizeof(value), "%02u:%02u", d.state.datetime.hour, d.state.datetime.minute);
        set(v.labels[Time], value);
        snprintf(value, sizeof(value), "%02u / %02u / %u", d.state.datetime.day,
                 d.state.datetime.month, d.state.datetime.year);
    } else {
        set(v.labels[Time], "--:--");
        snprintf(value, sizeof(value), "Hora sin sincronizar");
    }
    set(v.labels[Date], value);
    if (d.heart.is_valid) snprintf(value, sizeof(value), "%.0f", d.heart.bpm);
    else snprintf(value, sizeof(value), "--");
    set_value(v.labels[Pulse], value, VALUE_FONT);
    set(v.labels[PulseCaption], d.heart.is_valid ? PULSE_TEXT : NO_READING);
    if (d.temperature.is_valid) snprintf(value, sizeof(value), "%.1f", d.temperature.celsius);
    else snprintf(value, sizeof(value), "--");
    set_value(v.labels[Temperature], value, VALUE_FONT);
    set(v.labels[TempCaption], d.temperature.is_valid ? TEMP_TEXT : NO_READING);
    const unsigned slept = d.sleep.light_minutes + d.sleep.deep_minutes;
    if (d.uses_mock_data) snprintf(value, sizeof(value), "%uh %02um", slept / 60, slept % 60);
    else snprintf(value, sizeof(value), "--");
    set_value(v.labels[Sleep], value, SECONDARY_FONT);
    set(v.labels[SleepCaption], d.uses_mock_data ? SLEEP_TEXT : NO_SLEEP);
    const auto connection_color = lv_color_hex(d.state.ble_state == NERA_BLE_CONNECTED ? accent : muted);
    if (lv_color_to32(lv_obj_get_style_text_color(v.labels[Bluetooth], 0)) != lv_color_to32(connection_color))
        lv_obj_set_style_text_color(v.labels[Bluetooth], connection_color, 0);
    set(v.labels[Demo], d.uses_mock_data ? "Demo" : "");
}
