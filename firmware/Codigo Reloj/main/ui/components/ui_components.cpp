#include "ui/components/ui_components.h"

#include "nera_config.h"

namespace nera_ui {

void apply_screen_base_style(lv_obj_t *screen)
{
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(NERA_COLOR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
}

lv_obj_t *create_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(label, 0, LV_PART_MAIN);
    return label;
}

lv_obj_t *create_surface(lv_obj_t *parent, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *surface = lv_obj_create(parent);
    lv_obj_remove_style_all(surface);
    lv_obj_set_size(surface, width, height);
    lv_obj_set_style_bg_color(surface, lv_color_hex(NERA_COLOR_SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(surface, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(surface, NERA_UI_CORNER_RADIUS, LV_PART_MAIN);
    // Una superficie decorativa deja pasar el toque a la pantalla navegable.
    lv_obj_clear_flag(surface, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return surface;
}

lv_obj_t *create_metric_tile(lv_obj_t *parent, const char *title,
                             uint32_t accent, lv_obj_t **value_label)
{
    lv_obj_t *tile = create_surface(parent, NERA_UI_METRIC_WIDTH, NERA_UI_METRIC_HEIGHT);
    lv_obj_set_style_border_width(tile, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(tile, lv_color_hex(accent), LV_PART_MAIN);

    lv_obj_t *title_label = create_label(tile, &lv_font_montserrat_14,
                                         NERA_COLOR_TEXT_SECONDARY);
    lv_label_set_text(title_label, title);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 6);

    *value_label = create_label(tile, &lv_font_montserrat_14, accent);
    lv_obj_set_width(*value_label, NERA_UI_METRIC_WIDTH - 6);
    lv_obj_set_style_text_align(*value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(*value_label, LV_LABEL_LONG_CLIP);
    lv_label_set_text(*value_label, "--");
    lv_obj_align(*value_label, LV_ALIGN_BOTTOM_MID, 0, -8);
    return tile;
}

lv_obj_t *create_page_indicator(lv_obj_t *parent, uint8_t active_page, uint8_t page_count)
{
    if (page_count == 0 || active_page >= page_count) {
        return NULL;
    }

    const lv_coord_t width = page_count * NERA_UI_PAGE_DOT_SIZE +
                             (page_count - 1) * NERA_UI_PAGE_DOT_GAP;
    lv_obj_t *indicator = lv_obj_create(parent);
    lv_obj_remove_style_all(indicator);
    lv_obj_clear_flag(indicator, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(indicator, width, NERA_UI_PAGE_DOT_SIZE);
    lv_obj_align(indicator, LV_ALIGN_BOTTOM_MID, 0, -NERA_UI_PAGE_BOTTOM_MARGIN);

    for (uint8_t page = 0; page < page_count; ++page) {
        lv_obj_t *dot = lv_obj_create(indicator);
        lv_obj_remove_style_all(dot);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(dot, NERA_UI_PAGE_DOT_SIZE, NERA_UI_PAGE_DOT_SIZE);
        lv_obj_set_pos(dot, page * (NERA_UI_PAGE_DOT_SIZE + NERA_UI_PAGE_DOT_GAP), 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(dot, lv_color_hex(page == active_page ?
                                  NERA_COLOR_ACCENT : NERA_COLOR_TEXT_SECONDARY), LV_PART_MAIN);
    }
    return indicator;
}

}  // namespace nera_ui
