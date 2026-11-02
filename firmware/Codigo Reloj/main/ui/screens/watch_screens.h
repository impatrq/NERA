/** Contrato de vistas: cada modulo construye y actualiza contenido.
 * UIManager conserva los objetos y decide la navegacion en la tarea LVGL. */
#pragma once
#include "lvgl.h"
#include "services/health_service.h"

enum class WatchPage { Home, Health, Heart, Temperature, Sleep, Settings, Quick, Count };
// Las vistas declaran destinos; UIManager resuelve historial y transiciones.
void watch_navigate_on_tap(lv_obj_t *target, WatchPage page);
struct WatchView {
    lv_obj_t *root = nullptr;
    lv_obj_t *labels[12] = {};
    lv_obj_t *chart = nullptr;
    lv_chart_series_t *series = nullptr;
    float last_values[NERA_HISTORY_BUFFER_SIZE] = {};
    uint8_t last_count = 255;
    lv_obj_t *controls[4] = {};
};

void watchface_create(WatchView &view);
void watchface_update(WatchView &view, const NeraHealthSnapshot &data);
void health_screen_create(WatchView &view);
void health_screen_update(WatchView &view, const NeraHealthSnapshot &data);
void metric_screen_create(WatchView &view, bool temperature);
void metric_screen_update(WatchView &view, const NeraHealthSnapshot &data, bool temperature);
void sleep_screen_create(WatchView &view);
void sleep_screen_update(WatchView &view, const NeraHealthSnapshot &data);
void settings_screen_create(WatchView &view, bool quick = false);
void settings_screen_update(WatchView &view);
