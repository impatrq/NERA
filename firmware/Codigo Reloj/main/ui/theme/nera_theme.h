/** Tema NERA: colores, tipografia y primitivas compartidas por las pantallas.
 * Usa LVGL existente. No contiene sensores, navegacion ni almacenamiento. */
#pragma once
#include "lvgl.h"
#include <string.h>

namespace nera_theme {
constexpr uint32_t bg = 0x080A0B, surface = 0x191D20, text = 0xF5F7F8;
constexpr uint32_t muted = 0xADB4B9, accent = 0x92CDBD, heart = 0xDA94A0;
constexpr uint32_t temp = 0xD4BE95, sleep = 0xA7BAD4, line = 0x333B40;
// UTF-8 explicito: el glifo de grados ya esta en las fuentes Montserrat.
constexpr const char *degrees = "\xC2\xB0" "C";
constexpr int margin = 16;

// Solo modifica el texto cuando cambia, evitando invalidaciones innecesarias.
inline void set(lv_obj_t *label, const char *value) {
    if (strcmp(lv_label_get_text(label), value) != 0) lv_label_set_text(label, value);
}
inline lv_obj_t *label(lv_obj_t *parent, int x, int y, int width,
                       const lv_font_t *font, uint32_t color, const char *value = "") {
    lv_obj_t *obj = lv_label_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_width(obj, width);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_letter_space(obj, 0, 0);
    lv_label_set_text(obj, value);
    return obj;
}
inline lv_obj_t *panel(lv_obj_t *parent, int x, int y, int width, int height) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, width, height);
    lv_obj_set_style_bg_color(obj, lv_color_hex(surface), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, 8, 0);
    return obj;
}
// Fila tactil compartida. El valor domina; etiqueta y unidad no crean otro panel.
inline lv_obj_t *metric_row(lv_obj_t *parent, int y, int width, int height,
                            const char *name, lv_obj_t **value) {
    auto *row = panel(parent, margin, y, width, height);
    label(row, 10, (height - 16) / 2, 58, &lv_font_montserrat_14, muted, name);
    *value = label(row, 70, (height - 32) / 2, width - 80,
                   &lv_font_montserrat_28, text, "--");
    lv_label_set_long_mode(*value, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_align(*value, LV_TEXT_ALIGN_RIGHT, 0);
    return row;
}
inline void touch_feedback(lv_obj_t *obj) {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(line), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_STATE_PRESSED);
}
} // namespace nera_theme
