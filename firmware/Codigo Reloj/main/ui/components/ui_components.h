#pragma once

#include "lvgl.h"

// Todos los componentes se crean desde la tarea LVGL. Su padre es propietario
// de los objetos: al eliminar una pantalla, LVGL elimina tambien sus hijos.
namespace nera_ui {

void apply_screen_base_style(lv_obj_t *screen);
lv_obj_t *create_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color);
lv_obj_t *create_surface(lv_obj_t *parent, lv_coord_t width, lv_coord_t height);

// value_label permite actualizar la medicion sin reconstruir la tarjeta.
lv_obj_t *create_metric_tile(lv_obj_t *parent, const char *title,
                             uint32_t accent, lv_obj_t **value_label);

// active_page empieza en cero. El indicador es informativo, no un boton.
lv_obj_t *create_page_indicator(lv_obj_t *parent, uint8_t active_page, uint8_t page_count);

}  // namespace nera_ui
