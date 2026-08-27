#pragma once

#include "esp_err.h"
#include "lvgl.h"

esp_err_t nera_display_inicializar(void);
lv_display_t *nera_display_obtener(void);
