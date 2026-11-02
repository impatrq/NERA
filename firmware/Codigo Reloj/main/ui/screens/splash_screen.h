#pragma once

#include "esp_err.h"
#include "lvgl.h"

// Llamar una sola vez desde la tarea LVGL, con la watchface ya creada.
// La pantalla temporal se libera al terminar la transicion.
esp_err_t splash_screen_show(lv_obj_t *watch_screen);
