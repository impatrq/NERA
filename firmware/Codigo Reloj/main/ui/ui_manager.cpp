/** Navegacion LVGL: carrusel horizontal, accesos con retorno y panel rapido.
 * Las vistas solo declaran destinos. Todas las acciones y actualizaciones
 * se resuelven en la tarea grafica, sin cambiar servicios ni drivers. */
#include "ui/ui_manager.h"
#include "ui/screens/watch_screens.h"
#include "ui/screens/splash_screen.h"
#include "ui/theme/nera_theme.h"
#include "power/power_manager.h"
#include "core/event_bus.h"
#include "utils/logger.h"
#include <stdint.h>

namespace {
constexpr unsigned count = (unsigned)WatchPage::Count;
constexpr unsigned main_count = (unsigned)WatchPage::Quick;
WatchView views[count];
WatchPage current = WatchPage::Home;
WatchPage trail[8];
unsigned depth = 0;
uint32_t last_navigation = 0;
bool transitioning = true;
int directions[] = {-1, 1};

void update(lv_timer_t *) {
    NeraEvent event;
    for (unsigned i = 0; i < 16 && event_bus_receive(&event, 0) == ESP_OK; ++i) {}
    power_manager_tick(lv_disp_get_inactive_time(nullptr));
    NeraHealthSnapshot data = {};
    if (health_service_get_snapshot(&data) != ESP_OK) return;
    auto &view = views[(unsigned)current];
    switch (current) {
        case WatchPage::Home: watchface_update(view, data); break;
        case WatchPage::Health: health_screen_update(view, data); break;
        case WatchPage::Heart: metric_screen_update(view, data, false); break;
        case WatchPage::Temperature: metric_screen_update(view, data, true); break;
        case WatchPage::Sleep: sleep_screen_update(view, data); break;
        case WatchPage::Settings:
        case WatchPage::Quick: settings_screen_update(view); break;
        default: break;
    }
}
bool ready() {
    return !transitioning && lv_scr_act() == views[(unsigned)current].root &&
           lv_tick_elaps(last_navigation) >= NERA_UI_TRANSITION_MS;
}
void loaded(lv_event_t *) { transitioning = false; }
// El historial solo cambia una vez aceptada la accion. No guarda eventos en vuelo.
void open(WatchPage target, lv_scr_load_anim_t animation, bool remember) {
    if (!ready() || target == current || target >= WatchPage::Count) return;
    if (remember) {
        if (depth == 8) {
            for (unsigned i = 1; i < depth; ++i) trail[i - 1] = trail[i];
            --depth;
        }
        trail[depth++] = current;
    }
    current = target;
    transitioning = true;
    last_navigation = lv_tick_get();
    update(nullptr);
    lv_scr_load_anim(views[(unsigned)current].root, animation, NERA_UI_TRANSITION_MS, 0, false);
}
void back(lv_event_t *) {
    if (!ready()) return;
    if (current == WatchPage::Home) {
        open(WatchPage::Quick, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, true);
        return;
    }
    const WatchPage target = depth ? trail[--depth] : WatchPage::Home;
    open(target, LV_SCR_LOAD_ANIM_MOVE_RIGHT, false);
}
void navigate(int direction) {
    if (!ready() || current == WatchPage::Quick) return;
    const unsigned next = ((unsigned)current + main_count + direction) % main_count;
    depth = 0; // Deslizar recorre paginas principales; Volver queda apuntando al inicio.
    open((WatchPage)next, direction > 0 ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_MOVE_RIGHT, false);
}
void arrow(lv_event_t *event) { navigate(*static_cast<int *>(lv_event_get_user_data(event))); }
void shortcut(lv_event_t *event) {
    open((WatchPage)(uintptr_t)lv_event_get_user_data(event), LV_SCR_LOAD_ANIM_MOVE_LEFT, true);
}
void gesture(lv_event_t *) {
    auto *input = lv_indev_get_act();
    if (!input || !ready()) return;
    const lv_dir_t direction = lv_indev_get_gesture_dir(input);
    if (direction == LV_DIR_NONE) return;
    // Impide que el mismo deslizamiento termine activando la metrica bajo el dedo.
    lv_indev_wait_release(input);
    if (direction == LV_DIR_LEFT || direction == LV_DIR_RIGHT)
        navigate(direction == LV_DIR_LEFT ? 1 : -1);
    else if (direction == LV_DIR_BOTTOM)
        open(WatchPage::Quick, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, true);
    else if (direction == LV_DIR_TOP) {
        if (current == WatchPage::Quick) back(nullptr);
        else open(WatchPage::Health, LV_SCR_LOAD_ANIM_MOVE_TOP, true);
    }
}
lv_obj_t *footer_button(lv_obj_t *root, int x, int y, int width, const char *symbol,
                        lv_event_cb_t callback, void *data) {
    using namespace nera_theme;
    auto *button = panel(root, x, y, width, 44);
    lv_obj_set_style_bg_color(button, lv_color_hex(bg), 0);
    touch_feedback(button);
    lv_obj_add_event_cb(button, callback, LV_EVENT_SHORT_CLICKED, data);
    auto *icon = label(button, 0, 0, width, &lv_font_montserrat_14, text, symbol);
    lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(icon);
    return button;
}
void footer(lv_obj_t *root, unsigned page, int w, int h) {
    using namespace nera_theme;
    if (page < main_count) {
        footer_button(root, 4, h - 44, 44, LV_SYMBOL_LEFT, arrow, &directions[0]);
        footer_button(root, w - 48, h - 44, 44, LV_SYMBOL_RIGHT, arrow, &directions[1]);
        for (unsigned i = 0; i < main_count; ++i) {
            auto *dot = panel(root, (w - 66) / 2 + i * 12, h - 51, 6, 4);
            lv_obj_set_style_bg_color(dot, lv_color_hex(i == page ? accent : line), 0);
            lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        }
    }
    footer_button(root, (w - 80) / 2, h - 44, 80,
                  page == 0 ? LV_SYMBOL_SETTINGS : "Volver", back, nullptr);
}
}
void watch_navigate_on_tap(lv_obj_t *target, WatchPage page) {
    nera_theme::touch_feedback(target);
    lv_obj_add_event_cb(target, shortcut, LV_EVENT_SHORT_CLICKED, (void *)(uintptr_t)page);
}
esp_err_t ui_manager_init(void) {
    const int w = lv_disp_get_hor_res(nullptr), h = lv_disp_get_ver_res(nullptr);
    if (w < 240 || h < 320) return ESP_ERR_NOT_SUPPORTED;
    const char *titles[] = {"", "Salud", "Pulso", "Temperatura", "Sueno", "Ajustes", "Panel rapido"};
    for (unsigned i = 0; i < count; ++i) {
        auto *root = lv_obj_create(nullptr);
        views[i].root = root;
        lv_obj_remove_style_all(root);
        lv_obj_set_size(root, w, h);
        lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(root, lv_color_hex(nera_theme::bg), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
        lv_obj_add_event_cb(root, gesture, LV_EVENT_GESTURE, nullptr);
        lv_obj_add_event_cb(root, loaded, LV_EVENT_SCREEN_LOADED, nullptr);
        if (i) nera_theme::label(root, 16, 16, w - 32, &lv_font_montserrat_20, nera_theme::text, titles[i]);
        footer(root, i, w, h);
    }
    watchface_create(views[0]);
    health_screen_create(views[1]);
    metric_screen_create(views[2], false);
    metric_screen_create(views[3], true);
    sleep_screen_create(views[4]);
    settings_screen_create(views[5]);
    settings_screen_create(views[6], true);
    update(nullptr);
    if (splash_screen_show(views[0].root) != ESP_OK) {
        lv_scr_load(views[0].root);
        transitioning = false;
    }
    if (!lv_timer_create(update, 500, nullptr)) return ESP_ERR_NO_MEM;
    lv_mem_monitor_t memory;
    lv_mem_monitor(&memory);
    NERA_LOGI(NERA_TAG_LVGL, "NERA UI: 6 paginas + panel rapido; heap libre %u", (unsigned)memory.free_size);
    return ESP_OK;
}
