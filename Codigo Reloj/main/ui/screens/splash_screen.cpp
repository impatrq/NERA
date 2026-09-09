#include "ui/screens/splash_screen.h"
#include "ui/components/ui_components.h"

#include "nera_config.h"
#include "utils/logger.h"

static void finish_splash_cb(lv_timer_t *timer)
{
    lv_obj_t *watch_screen = static_cast<lv_obj_t *>(timer->user_data);
    // auto_del libera solamente el splash anterior, no la watchface.
    lv_scr_load_anim(watch_screen, LV_SCR_LOAD_ANIM_FADE_IN,
                     NERA_UI_TRANSITION_MS, 0, true);
    NERA_LOGI(NERA_TAG_LVGL, "Splash finished; opening watchface");
}

esp_err_t splash_screen_show(lv_obj_t *watch_screen)
{
    if (watch_screen == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    lv_obj_t *screen = lv_obj_create(NULL);
    nera_ui::apply_screen_base_style(screen);
    lv_obj_set_style_pad_all(screen, 0, 0);

    const lv_coord_t content_width = lv_disp_get_hor_res(NULL) - 32;
    lv_obj_t *brand = nera_ui::create_label(screen, &lv_font_montserrat_28, NERA_COLOR_ACCENT);
    lv_label_set_text(brand, "NERA");
    lv_obj_set_width(brand, content_width);
    lv_obj_set_style_text_align(brand, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(brand, LV_ALIGN_CENTER, 0, -24);

    lv_obj_t *status = nera_ui::create_label(screen, &lv_font_montserrat_14,
                                           NERA_COLOR_TEXT_SECONDARY);
    lv_label_set_text(status, "Inicializando sensores...");
    lv_obj_set_width(status, content_width);
    lv_label_set_long_mode(status, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(status, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(status, LV_ALIGN_CENTER, 0, 24);

    // Un timer de una ejecucion se elimina solo. No detiene las tareas de sensores.
    lv_timer_t *timer = lv_timer_create(finish_splash_cb, NERA_UI_SPLASH_DURATION_MS,
                                        watch_screen);
    if (timer == NULL) {
        lv_obj_del(screen);
        return ESP_ERR_NO_MEM;
    }
    lv_timer_set_repeat_count(timer, 1);
    lv_scr_load(screen);
    lv_obj_fade_in(brand, NERA_UI_TRANSITION_MS, 0);
    lv_obj_fade_in(status, NERA_UI_TRANSITION_MS, NERA_UI_SPLASH_STATUS_DELAY_MS);
    return ESP_OK;
}
