/** Pantalla principal: presenta snapshots reales sin acceder al bus I2C.
 * Todos los objetos LVGL se crean y actualizan en la tarea grafica. */
#include "ui/ui_manager.h"
#include "display/display_manager.h"
#include "sensor_service.h"
#include "services/time_service.h"
#include "nera_config.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>

namespace {
// Posiciones de la vista existente, con ambiente en el espacio del boton demo.
constexpr uint32_t BACKGROUND = 0x080B12, TEXT = 0xF2F4F8, MUTED = 0x9BA6B7;
constexpr uint32_t ACCENT = 0x57BDD1, AMBER = 0xE8B766;
constexpr int MARGIN = 20, CONTENT_WIDTH = NERA_LCD_WIDTH - 2 * MARGIN;
constexpr int PULSE_Y = 82, OBJECT_Y = 143, MOTION_Y = 204, AMBIENT_Y = 252;
constexpr uint32_t REFRESH_MS = 250;

lv_obj_t *pulse_value = nullptr, *object_value = nullptr;
lv_obj_t *pulse_caption = nullptr;
lv_obj_t *oxygen_caption = nullptr, *oxygen_value = nullptr;
lv_obj_t *motion_value = nullptr, *ambient_value = nullptr;
lv_obj_t *clock_value = nullptr;

lv_obj_t *label(lv_obj_t *parent, const char *text, int y,
                const lv_font_t *font, uint32_t color) {
    auto *object = lv_label_create(parent);
    lv_label_set_text(object, text);
    lv_obj_set_width(object, CONTENT_WIDTH);
    lv_obj_set_pos(object, MARGIN, y);
    lv_obj_set_style_text_font(object, font, 0);
    lv_obj_set_style_text_color(object, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(object, LV_TEXT_ALIGN_LEFT, 0);
    return object;
}

void render_sensor_snapshot(lv_timer_t *) {
    char clock_text[6] = "--:--";
    time_service_local_hhmm(clock_text);
    if (strcmp(lv_label_get_text(clock_value), clock_text) != 0)
        lv_label_set_text(clock_value, clock_text);
    SensorSnapshot sample = {};
    if (sensor_service_get_snapshot(&sample) != ESP_OK) sample = {};
    char value[48];

    if (sample.heart_valid || sample.bpm > 0) {
        // La ultima cifra sigue visible entre detecciones; la leyenda indica
        // si ya no es una lectura actual. BLE e historial conservan su validez.
        lv_label_set_text(pulse_caption, sample.heart_valid && !sample.heart_held ?
                          "Pulso" : "Ultimo pulso");
        lv_obj_set_style_text_font(pulse_value,&lv_font_montserrat_20,0);
        lv_obj_set_style_text_color(pulse_value,
                                    lv_color_hex(sample.heart_valid && !sample.heart_held ?
                                                 0xE88996 : MUTED), 0);
        snprintf(value, sizeof(value), "%.0f BPM", sample.bpm);
        lv_label_set_text(pulse_value, value);
    } else {
        lv_label_set_text(pulse_caption,"Pulso");
        lv_obj_set_style_text_font(pulse_value,&lv_font_montserrat_14,0);
        lv_obj_set_style_text_color(pulse_value,lv_color_hex(0xE88996),0);
        const char *state=!sample.max_online ? "MAX sin conexion" :
            sample.max_saturated ? "Senal saturada" :
            !sample.max_contact ? "Apoya el dedo" : "Buscando pulso";
        lv_label_set_text(pulse_value,state);
    }
    if (sample.spo2_valid || sample.spo2 > 0) {
        lv_label_set_text(oxygen_caption, sample.spo2_valid ? "SpO2 est." : "Ult. SpO2");
        lv_obj_set_style_text_color(oxygen_value,
                                    lv_color_hex(sample.spo2_valid ? ACCENT : MUTED), 0);
        snprintf(value, sizeof(value), "%.0f%%", sample.spo2);
        lv_label_set_text(oxygen_value, value);
    } else {
        lv_label_set_text(oxygen_caption, "SpO2 est.");
        lv_obj_set_style_text_color(oxygen_value, lv_color_hex(ACCENT), 0);
        lv_label_set_text(oxygen_value, "--");
    }

    if (sample.mlx_valid) {
        snprintf(value, sizeof(value), "%.1f °C", sample.object_celsius);
        lv_label_set_text(object_value, value);
        snprintf(value, sizeof(value), "%.1f °C", sample.ambient_celsius);
        lv_label_set_text(ambient_value, value);
    } else {
        lv_label_set_text(object_value, "Sin lectura");
        lv_label_set_text(ambient_value, "Sin lectura");
    }

    if (sample.mpu_valid) {
        // Son magnitudes medidas. La aceleracion incluye gravedad; no se
        // interpreta como un diagnostico ni como una fase de sueno.
        snprintf(value, sizeof(value), "A %.2fg  G %.0f°/s",
                 sample.acceleration_g, sample.rotation_dps);
        lv_point_t extent;
        lv_txt_get_size(&extent, value, &lv_font_montserrat_20, 0, 0,
                        LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        lv_obj_set_style_text_font(motion_value,
            extent.x <= CONTENT_WIDTH ? &lv_font_montserrat_20 : &lv_font_montserrat_14, 0);
        lv_label_set_text(motion_value, value);
    } else {
        lv_obj_set_style_text_font(motion_value, &lv_font_montserrat_20, 0);
        lv_label_set_text(motion_value, "Sin lectura");
    }
}

void on_brightness(lv_event_t *event) {
    const auto value = lv_slider_get_value(lv_event_get_target(event));
    display_manager_set_brightness((uint8_t)value);
}
}

esp_err_t ui_manager_init(void) {
    auto *screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(BACKGROUND), 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    auto *title = label(screen, "NERA", 18, &lv_font_montserrat_28, TEXT);
    lv_obj_set_width(title, 100);
    clock_value = label(screen, "--:--", 21, &lv_font_montserrat_20, AMBER);
    lv_obj_set_pos(clock_value, 135, 21);
    lv_obj_set_width(clock_value, 85);
    lv_obj_set_style_text_align(clock_value, LV_TEXT_ALIGN_RIGHT, 0);
    label(screen, "Lecturas de sensores", 56, LV_FONT_DEFAULT, MUTED);

    pulse_caption=label(screen, "Pulso", PULSE_Y, LV_FONT_DEFAULT, MUTED);
    lv_obj_set_width(pulse_caption, 112);
    pulse_value = label(screen, "Sin lectura", PULSE_Y + 18, &lv_font_montserrat_20, 0xE88996);
    lv_obj_set_width(pulse_value, 112);
    lv_label_set_long_mode(pulse_value, LV_LABEL_LONG_CLIP);
    oxygen_caption = label(screen, "SpO2 est.", PULSE_Y, LV_FONT_DEFAULT, MUTED);
    lv_obj_set_pos(oxygen_caption, 139, PULSE_Y);
    lv_obj_set_width(oxygen_caption, 81);
    oxygen_value = label(screen, "--", PULSE_Y + 18, &lv_font_montserrat_28, ACCENT);
    lv_obj_set_pos(oxygen_value, 139, PULSE_Y + 18);
    lv_obj_set_width(oxygen_value, 81);

    label(screen, "Objeto IR sin contacto", OBJECT_Y, LV_FONT_DEFAULT, MUTED);
    object_value = label(screen, "Sin lectura", OBJECT_Y + 18, &lv_font_montserrat_28, AMBER);

    // Un rotulo corto evita el salto de linea que se superponia al dato.
    label(screen, "Movimiento", MOTION_Y, LV_FONT_DEFAULT, MUTED);
    motion_value = label(screen, "Sin lectura", MOTION_Y + 20, &lv_font_montserrat_20, ACCENT);
    lv_label_set_long_mode(motion_value, LV_LABEL_LONG_CLIP);
    lv_obj_set_height(motion_value, 26);

    label(screen, "Temperatura ambiente", AMBIENT_Y, LV_FONT_DEFAULT, MUTED);
    ambient_value = label(screen, "Sin lectura", AMBIENT_Y + 17, &lv_font_montserrat_20, AMBER);

    auto *brightness_label = label(screen, "Brillo", 297, LV_FONT_DEFAULT, MUTED);
    lv_obj_set_width(brightness_label, 50);
    auto *slider = lv_slider_create(screen);
    lv_obj_set_pos(slider, 90, 303);
    lv_obj_set_size(slider, 125, 6);
    lv_slider_set_range(slider, NERA_BL_MIN, NERA_BL_MAX);
    lv_slider_set_value(slider, display_manager_get_brightness(), LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(ACCENT), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider, on_brightness, LV_EVENT_VALUE_CHANGED, nullptr);

    render_sensor_snapshot(nullptr);
    lv_timer_create(render_sensor_snapshot, REFRESH_MS, nullptr);
    lv_scr_load(screen);
    return ESP_OK;
}
