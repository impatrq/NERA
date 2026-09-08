/**
 * @file ui_manager.cpp
 * @brief Watchface y navegacion basica de NERA sobre LVGL.
 *
 * Esta etapa crea una base simple pero ampliable:
 * - Pantalla Watch: informacion principal visible de un vistazo.
 * - Pantalla Health: resumen de salud y grafico historico de BPM.
 * - Navegacion por toque o gesto usando eventos de LVGL.
 */

#include "ui/ui_manager.h"
#include "ui/components/ui_components.h"
#include "ui/screens/splash_screen.h"

#include <stdio.h>

#include "core/app_state.h"
#include "services/health_service.h"
#include "services/sleep_service.h"
#include "power/power_manager.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "nera_config.h"
#include "utils/logger.h"

static const char *TAG = NERA_TAG_LVGL;

using nera_ui::apply_screen_base_style;
using nera_ui::create_label;
using nera_ui::create_metric_tile;

typedef enum {
    NERA_UI_SCREEN_WATCH = 0,
    NERA_UI_SCREEN_HEALTH = 1,
    NERA_UI_SCREEN_HEART = 2,
    NERA_UI_SCREEN_TEMP = 3,
    NERA_UI_SCREEN_SLEEP = 4,
    NERA_UI_SCREEN_SETTINGS = 5,
    NERA_UI_SCREEN_COUNT = 6,
} NeraUIScreen;

static lv_obj_t *s_watch_screen = NULL;
static lv_obj_t *s_health_screen = NULL;
static lv_obj_t *s_screens[NERA_UI_SCREEN_COUNT] = {};

static lv_obj_t *s_watch_time_label = NULL;
static lv_obj_t *s_watch_date_label = NULL;
static lv_obj_t *s_watch_status_label = NULL;
static lv_obj_t *s_watch_heart_value = NULL;
static lv_obj_t *s_watch_temp_value = NULL;
static lv_obj_t *s_watch_battery_value = NULL;

static lv_obj_t *s_health_summary_label = NULL;
static lv_obj_t *s_health_heart_value = NULL;
static lv_obj_t *s_health_temp_value = NULL;
static lv_obj_t *s_health_battery_value = NULL;
static lv_obj_t *s_health_chart = NULL;
static lv_chart_series_t *s_health_heart_series = NULL;

static lv_obj_t *s_heart_current = NULL;
static lv_obj_t *s_heart_min = NULL;
static lv_obj_t *s_heart_max = NULL;
static lv_obj_t *s_heart_avg = NULL;
static lv_obj_t *s_heart_status = NULL;
static lv_obj_t *s_temp_current = NULL;
static lv_obj_t *s_temp_trend = NULL;
static lv_obj_t *s_temp_source = NULL;
static lv_obj_t *s_sleep_summary = NULL;
static lv_obj_t *s_sleep_details = NULL;
static lv_obj_t *s_settings_brightness = NULL;
static lv_obj_t *s_settings_power = NULL;
static lv_obj_t *s_settings_info = NULL;

static NeraUIScreen s_current_screen = NERA_UI_SCREEN_WATCH;

static const char *weekday_name(uint8_t weekday)
{
    static const char *names[] = {
        "DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"
    };
    return names[weekday <= 6 ? weekday : 0];
}

static const char *mode_label(NeraSystemMode mode)
{
    switch (mode) {
        case NERA_MODE_READY:
            return "LISTO";
        case NERA_MODE_MEASURING:
            return "MIDIENDO";
        case NERA_MODE_LOW_BATTERY:
            return "BATERIA BAJA";
        case NERA_MODE_CHARGING:
            return "CARGANDO";
        case NERA_MODE_SLEEP:
            return "SLEEP";
        case NERA_MODE_ERROR:
            return "ERROR";
        default:
            return "INICIANDO";
    }
}

static void show_screen(NeraUIScreen screen)
{
    lv_obj_t *target = (screen < NERA_UI_SCREEN_COUNT) ? s_screens[screen] : NULL;
    if (target == NULL || screen == s_current_screen) {
        return;
    }

    lv_scr_load_anim(target,
                     screen > s_current_screen ?
                     LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                     NERA_UI_TRANSITION_MS,
                     0,
                     false);
    s_current_screen = screen;
}

static void navigation_event_cb(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t gesture = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (gesture == LV_DIR_LEFT || gesture == LV_DIR_RIGHT) {
            const bool forward = gesture == LV_DIR_LEFT;
            const int next = forward ?
                (s_current_screen + 1) % NERA_UI_SCREEN_COUNT :
                (s_current_screen + NERA_UI_SCREEN_COUNT - 1) % NERA_UI_SCREEN_COUNT;
            show_screen((NeraUIScreen)next);
        }
        return;
    }

    if (code == LV_EVENT_CLICKED) {
        show_screen((NeraUIScreen)((s_current_screen + 1) % NERA_UI_SCREEN_COUNT));
    }
}

static void create_watch_screen(void)
{
    s_watch_screen = lv_obj_create(NULL);
    s_screens[NERA_UI_SCREEN_WATCH] = s_watch_screen;
    apply_screen_base_style(s_watch_screen);
    lv_obj_add_event_cb(s_watch_screen, navigation_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(s_watch_screen, navigation_event_cb, LV_EVENT_GESTURE, NULL);

    lv_obj_t *brand = create_label(s_watch_screen, &lv_font_montserrat_14,
                                   NERA_COLOR_ACCENT);
    lv_label_set_text(brand, "NERA");
    lv_obj_align(brand, LV_ALIGN_TOP_LEFT, NERA_UI_HEADER_MARGIN, 14);

    s_watch_battery_value = create_label(s_watch_screen, &lv_font_montserrat_14,
                                         NERA_COLOR_BATTERY);
    lv_obj_align(s_watch_battery_value, LV_ALIGN_TOP_RIGHT, -NERA_UI_HEADER_MARGIN, 14);

    s_watch_time_label = create_label(s_watch_screen, &lv_font_montserrat_28,
                                      NERA_COLOR_TEXT_PRIMARY);
    lv_obj_align(s_watch_time_label, LV_ALIGN_TOP_MID, 0, 48);

    s_watch_date_label = create_label(s_watch_screen, &lv_font_montserrat_14,
                                      NERA_COLOR_TEXT_SECONDARY);
    lv_obj_align(s_watch_date_label, LV_ALIGN_TOP_MID, 0, 92);

    lv_obj_t *metrics_card = nera_ui::create_surface(
        s_watch_screen, NERA_UI_CONTENT_WIDTH, NERA_UI_WATCH_CARD_HEIGHT);
    lv_obj_align(metrics_card, LV_ALIGN_TOP_MID, 0, NERA_UI_WATCH_CARD_TOP);

    s_watch_heart_value = create_label(metrics_card, &lv_font_montserrat_20,
                                       NERA_COLOR_HEART);
    lv_obj_align(s_watch_heart_value, LV_ALIGN_LEFT_MID, 16, -8);

    s_watch_temp_value = create_label(metrics_card, &lv_font_montserrat_20,
                                      NERA_COLOR_TEMP);
    lv_obj_align(s_watch_temp_value, LV_ALIGN_RIGHT_MID, -16, -6);

    s_watch_status_label = create_label(s_watch_screen, &lv_font_montserrat_14,
                                        NERA_COLOR_TEXT_SECONDARY);
    lv_obj_set_width(s_watch_status_label, NERA_UI_CONTENT_WIDTH);
    lv_obj_set_style_text_align(s_watch_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_watch_status_label, LV_ALIGN_TOP_MID, 0, 212);

    nera_ui::create_page_indicator(s_watch_screen, NERA_UI_SCREEN_WATCH,
                                   NERA_UI_SCREEN_COUNT);
}

static void create_health_screen(void)
{
    s_health_screen = lv_obj_create(NULL);
    s_screens[NERA_UI_SCREEN_HEALTH] = s_health_screen;
    apply_screen_base_style(s_health_screen);
    lv_obj_add_event_cb(s_health_screen, navigation_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(s_health_screen, navigation_event_cb, LV_EVENT_GESTURE, NULL);

    lv_obj_t *title = create_label(s_health_screen, &lv_font_montserrat_20,
                                   NERA_COLOR_TEXT_PRIMARY);
    lv_label_set_text(title, "Salud");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, NERA_UI_HEADER_MARGIN, 16);

    s_health_summary_label = create_label(s_health_screen, &lv_font_montserrat_14,
                                          NERA_COLOR_TEXT_SECONDARY);
    lv_label_set_long_mode(s_health_summary_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_health_summary_label, NERA_UI_CONTENT_WIDTH - 72);
    lv_obj_align(s_health_summary_label, LV_ALIGN_TOP_RIGHT, -NERA_UI_HEADER_MARGIN, 18);

    lv_obj_t *tile_heart = create_metric_tile(s_health_screen, "BPM",
                                              NERA_COLOR_HEART,
                                              &s_health_heart_value);
    lv_obj_align(tile_heart, LV_ALIGN_TOP_LEFT, NERA_UI_SIDE_MARGIN, NERA_UI_HEALTH_TILES_TOP);

    lv_obj_t *tile_temp = create_metric_tile(s_health_screen, "TEMP",
                                             NERA_COLOR_TEMP,
                                             &s_health_temp_value);
    lv_obj_align(tile_temp, LV_ALIGN_TOP_MID, 0, NERA_UI_HEALTH_TILES_TOP);

    lv_obj_t *tile_battery = create_metric_tile(s_health_screen, "BAT",
                                                NERA_COLOR_BATTERY,
                                                &s_health_battery_value);
    lv_obj_align(tile_battery, LV_ALIGN_TOP_RIGHT, -NERA_UI_SIDE_MARGIN,
                 NERA_UI_HEALTH_TILES_TOP);

    s_health_chart = lv_chart_create(s_health_screen);
    lv_obj_set_size(s_health_chart, NERA_UI_CONTENT_WIDTH, NERA_UI_HEALTH_CHART_HEIGHT);
    lv_obj_align(s_health_chart, LV_ALIGN_BOTTOM_MID, 0, -NERA_UI_HEALTH_CHART_BOTTOM);
    lv_obj_set_style_bg_color(s_health_chart, lv_color_hex(NERA_COLOR_SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_health_chart, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(s_health_chart, NERA_UI_CORNER_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_health_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_color(s_health_chart, lv_color_hex(0x303044), LV_PART_MAIN);
    lv_chart_set_type(s_health_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_health_chart, NERA_HISTORY_BUFFER_SIZE);
    lv_chart_set_range(s_health_chart, LV_CHART_AXIS_PRIMARY_Y, 50, 110);
    s_health_heart_series = lv_chart_add_series(
        s_health_chart,
        lv_color_hex(NERA_COLOR_HEART),
        LV_CHART_AXIS_PRIMARY_Y);

    nera_ui::create_page_indicator(s_health_screen, NERA_UI_SCREEN_HEALTH,
                                   NERA_UI_SCREEN_COUNT);
}

static lv_obj_t *create_detail_screen(const char *title, NeraUIScreen screen)
{
    lv_obj_t *root = lv_obj_create(NULL);
    s_screens[screen] = root;
    apply_screen_base_style(root);
    lv_obj_add_event_cb(root, navigation_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(root, navigation_event_cb, LV_EVENT_GESTURE, NULL);

    lv_obj_t *heading = create_label(root, &lv_font_montserrat_20,
                                     NERA_COLOR_TEXT_PRIMARY);
    lv_label_set_text(heading, title);
    lv_obj_align(heading, LV_ALIGN_TOP_LEFT, NERA_UI_HEADER_MARGIN, 16);
    nera_ui::create_page_indicator(root, screen, NERA_UI_SCREEN_COUNT);
    return root;
}

static void create_heart_screen(void)
{
    lv_obj_t *root = create_detail_screen("Corazon", NERA_UI_SCREEN_HEART);

    s_heart_current = create_label(root, &lv_font_montserrat_28, NERA_COLOR_HEART);
    lv_obj_align(s_heart_current, LV_ALIGN_TOP_MID, 0, 56);
    s_heart_status = create_label(root, &lv_font_montserrat_14, NERA_COLOR_TEXT_SECONDARY);
    lv_obj_align(s_heart_status, LV_ALIGN_TOP_MID, 0, 100);

    lv_obj_t *min_tile = create_metric_tile(root, "MIN", NERA_COLOR_INFO, &s_heart_min);
    lv_obj_align(min_tile, LV_ALIGN_TOP_LEFT, NERA_UI_SIDE_MARGIN, 142);
    lv_obj_t *max_tile = create_metric_tile(root, "MAX", NERA_COLOR_WARNING, &s_heart_max);
    lv_obj_align(max_tile, LV_ALIGN_TOP_MID, 0, 142);
    lv_obj_t *avg_tile = create_metric_tile(root, "AVG", NERA_COLOR_HEALTH_OK, &s_heart_avg);
    lv_obj_align(avg_tile, LV_ALIGN_TOP_RIGHT, -NERA_UI_SIDE_MARGIN, 142);
}

static void create_temperature_screen(void)
{
    lv_obj_t *root = create_detail_screen("Temperatura", NERA_UI_SCREEN_TEMP);

    s_temp_current = create_label(root, &lv_font_montserrat_28, NERA_COLOR_TEMP);
    lv_obj_align(s_temp_current, LV_ALIGN_TOP_MID, 0, 58);
    s_temp_trend = create_label(root, &lv_font_montserrat_14, NERA_COLOR_TEXT_SECONDARY);
    lv_obj_align(s_temp_trend, LV_ALIGN_TOP_MID, 0, 106);
    s_temp_source = create_label(root, &lv_font_montserrat_14, NERA_COLOR_WARNING);
    lv_obj_set_width(s_temp_source, NERA_UI_CONTENT_WIDTH);
    lv_obj_set_style_text_align(s_temp_source, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_temp_source, LV_ALIGN_TOP_MID, 0, 156);

    lv_obj_t *note = create_label(root, &lv_font_montserrat_14, NERA_COLOR_TEXT_DISABLED);
    lv_label_set_text(note, "No equivale a temperatura corporal");
    lv_obj_set_width(note, NERA_UI_CONTENT_WIDTH);
    lv_obj_set_style_text_align(note, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(note, LV_ALIGN_TOP_MID, 0, 198);
}

static void create_sleep_screen(void)
{
    lv_obj_t *root = create_detail_screen("Sueno", NERA_UI_SCREEN_SLEEP);
    s_sleep_summary = create_label(root, &lv_font_montserrat_28, NERA_COLOR_SLEEP);
    lv_obj_align(s_sleep_summary, LV_ALIGN_TOP_MID, 0, 58);
    s_sleep_details = create_label(root, &lv_font_montserrat_14, NERA_COLOR_TEXT_SECONDARY);
    lv_obj_set_width(s_sleep_details, NERA_UI_CONTENT_WIDTH);
    lv_obj_set_style_text_align(s_sleep_details, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(s_sleep_details, LV_LABEL_LONG_WRAP);
    lv_obj_align(s_sleep_details, LV_ALIGN_TOP_MID, 0, 112);
}

static void create_settings_screen(void)
{
    lv_obj_t *root = create_detail_screen("Ajustes", NERA_UI_SCREEN_SETTINGS);
    s_settings_brightness = create_label(root, &lv_font_montserrat_14,
                                         NERA_COLOR_TEXT_PRIMARY);
    lv_obj_align(s_settings_brightness, LV_ALIGN_TOP_LEFT, NERA_UI_SIDE_MARGIN, 70);
    s_settings_power = create_label(root, &lv_font_montserrat_14,
                                    NERA_COLOR_HEALTH_OK);
    lv_obj_align(s_settings_power, LV_ALIGN_TOP_LEFT, NERA_UI_SIDE_MARGIN, 112);
    s_settings_info = create_label(root, &lv_font_montserrat_14,
                                   NERA_COLOR_TEXT_SECONDARY);
    lv_obj_set_width(s_settings_info, NERA_UI_CONTENT_WIDTH);
    lv_label_set_long_mode(s_settings_info, LV_LABEL_LONG_WRAP);
    lv_obj_align(s_settings_info, LV_ALIGN_TOP_LEFT, NERA_UI_SIDE_MARGIN, 164);
}

static void update_health_chart(const NeraAppState *state)
{
    if (s_health_chart == NULL || s_health_heart_series == NULL || state == NULL) {
        return;
    }

    for (uint16_t index = 0; index < NERA_HISTORY_BUFFER_SIZE; ++index) {
        lv_coord_t point = LV_CHART_POINT_NONE;
        if (index < state->heart_history_count && state->heart_history[index] > 0.0f) {
            point = (lv_coord_t)state->heart_history[index];
        }
        lv_chart_set_value_by_id(s_health_chart, s_health_heart_series, index, point);
    }
    lv_chart_refresh(s_health_chart);
}

static void update_ui_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    NeraAppState state = {};
    if (app_state_get(&state) != ESP_OK) {
        return;
    }

    NeraHealthSnapshot health = {};
    if (health_service_get_snapshot(&health) != ESP_OK) {
        return;
    }
    NeraSleepData sleep = {};
    sleep_service_get_data(&sleep);

    const uint32_t total_seconds = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    const uint32_t hours = (total_seconds / 3600U) % 24U;
    const uint32_t minutes = (total_seconds / 60U) % 60U;

    char time_text[8];
    char date_text[24];
    char status_text[64];
    char heart_text[16];
    char temp_text[16];
    char battery_text[16];

    snprintf(time_text, sizeof(time_text), "%02lu:%02lu",
             (unsigned long)hours, (unsigned long)minutes);
    snprintf(date_text, sizeof(date_text), "%s %02u/%02u",
             weekday_name(state.datetime.weekday),
             (unsigned)state.datetime.day,
             (unsigned)state.datetime.month);
    snprintf(heart_text, sizeof(heart_text), health.heart.is_valid ? "%02d" : "--",
             (int)health.heart.bpm);
    snprintf(temp_text, sizeof(temp_text), health.temperature.is_valid ? "%.1f C" : "--.- C",
             health.temperature.celsius);
    snprintf(battery_text, sizeof(battery_text), "%u%%",
             (unsigned)state.battery.percentage);
    snprintf(status_text, sizeof(status_text), "%s  |  MOCK DATA",
             mode_label(state.mode));

    lv_label_set_text(s_watch_time_label, time_text);
    lv_label_set_text(s_watch_date_label, date_text);
    lv_label_set_text(s_watch_status_label, status_text);
    lv_label_set_text(s_watch_heart_value, heart_text);
    lv_label_set_text(s_watch_temp_value, temp_text);
    lv_label_set_text(s_watch_battery_value, battery_text);

    lv_label_set_text(s_health_heart_value, heart_text);
    lv_label_set_text(s_health_temp_value, temp_text);
    lv_label_set_text(s_health_battery_value, battery_text);

    char summary_text[80];
    snprintf(summary_text, sizeof(summary_text), "%s\n%s",
             health.overall_ok ? "Estado normal" : "Revisar sensores",
             health.uses_mock_data ? "Datos simulados" : "Datos medidos");
    lv_label_set_text(s_health_summary_label, summary_text);

    char detail_text[96];
    snprintf(detail_text, sizeof(detail_text), "%s BPM", heart_text);
    lv_label_set_text(s_heart_current, detail_text);
    snprintf(detail_text, sizeof(detail_text), "%02d BPM", (int)health.heart.bpm_min);
    lv_label_set_text(s_heart_min, detail_text);
    snprintf(detail_text, sizeof(detail_text), "%02d BPM", (int)health.heart.bpm_max);
    lv_label_set_text(s_heart_max, detail_text);
    snprintf(detail_text, sizeof(detail_text), "%02d BPM", (int)health.heart.bpm_avg);
    lv_label_set_text(s_heart_avg, detail_text);
    lv_label_set_text(s_heart_status, health.heart.is_valid ? "Medicion estable" : "Sensor no disponible");

    lv_label_set_text(s_temp_current, temp_text);
    lv_label_set_text(s_temp_trend, "Tendencia: estable");
    lv_label_set_text(s_temp_source, health.uses_mock_data ? "DEMO / MOCK SENSOR" : "SENSOR REAL");

    const uint16_t sleep_hours = sleep.duration_minutes / 60;
    const uint16_t sleep_minutes = sleep.duration_minutes % 60;
    snprintf(detail_text, sizeof(detail_text), "%uh %02um", sleep_hours, sleep_minutes);
    lv_label_set_text(s_sleep_summary, detail_text);
    snprintf(detail_text, sizeof(detail_text), "Calidad %u%%\nLigero %um  Profundo %um\nDespierto %um\n%s",
             sleep.quality_percent, sleep.light_minutes, sleep.deep_minutes,
             sleep.awake_minutes, sleep.is_estimate ? "ESTIMACION" : "MEDIDO");
    lv_label_set_text(s_sleep_details, detail_text);

    snprintf(detail_text, sizeof(detail_text), "Brillo: %u/255",
             power_manager_get_brightness());
    lv_label_set_text(s_settings_brightness, detail_text);
    lv_label_set_text(s_settings_power,
                      power_manager_is_saver_enabled() ? "Ahorro: activo" : "Ahorro: inactivo");
    lv_label_set_text(s_settings_info, "Notificaciones: preparadas\nVibracion: preparada\nBLE: arquitectura lista");

    update_health_chart(&state);
}

esp_err_t ui_manager_init(void)
{
    NERA_LOGI(TAG, "Initializing NERA UI manager...");

    create_watch_screen();
    create_health_screen();
    create_heart_screen();
    create_temperature_screen();
    create_sleep_screen();
    create_settings_screen();
    s_current_screen = NERA_UI_SCREEN_WATCH;

    update_ui_timer_cb(NULL);
    lv_timer_create(update_ui_timer_cb, 1000, NULL);

    esp_err_t splash_result = splash_screen_show(s_watch_screen);
    if (splash_result != ESP_OK) {
        NERA_LOGW(TAG, "Splash unavailable: %s", esp_err_to_name(splash_result));
        lv_scr_load(s_watch_screen);
    }

    NERA_LOGI(TAG, "UI manager ready: splash + watchface + health screen");
    return ESP_OK;
}
