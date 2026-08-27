#include "pantalla_principal.h"

#include <math.h>
#include <stdio.h>

#include "esp_check.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"

#define NERA_COLOR_PRIMARIO    0x0E9384
#define NERA_COLOR_SECUNDARIO  0x2563EB
#define NERA_COLOR_FONDO       0xF8FAFC
#define NERA_COLOR_TEXTO       0x0F172A
#define NERA_COLOR_PELIGRO     0xEF4444
#define NERA_COLOR_EXITO       0x10B981
#define NERA_COLOR_AMARILLO    0xF59E0B
#define NERA_COLOR_BORDE       0xE2E8F0

static lv_obj_t *pantalla;
static lv_obj_t *label_ble;
static lv_obj_t *label_hora;
static lv_obj_t *arco_bienestar;
static lv_obj_t *label_bienestar;
static lv_obj_t *label_estado;
static lv_obj_t *label_hr;
static lv_obj_t *label_spo2;
static lv_obj_t *label_temp;
static lv_obj_t *label_mov;

static int limitar_entero(int valor, int minimo, int maximo)
{
    if (valor < minimo) {
        return minimo;
    }
    if (valor > maximo) {
        return maximo;
    }
    return valor;
}

static int calcular_estres_estimado(const nera_metricas_reloj_t *m)
{
    const float movimiento = fabsf(m->ax) + fabsf(m->ay) + fabsf(m->az - 1.0f);
    const float aporte_pulso = fmaxf(0.0f, ((float)m->hr - 65.0f) * 1.4f);
    const float aporte_mov = fmaxf(0.0f, (movimiento - 1.0f) * 18.0f);
    const float aporte_temp = fmaxf(0.0f, (m->temp - 36.7f) * 22.0f);
    return limitar_entero((int)lroundf(aporte_pulso + aporte_mov + aporte_temp), 0, 100);
}

static lv_color_t color_estres(int estres)
{
    if (estres < 35) {
        return lv_color_hex(NERA_COLOR_EXITO);
    }
    if (estres <= 65) {
        return lv_color_hex(NERA_COLOR_AMARILLO);
    }
    return lv_color_hex(NERA_COLOR_PELIGRO);
}

static lv_obj_t *crear_label(lv_obj_t *padre, const char *texto, int tam, uint32_t color)
{
    lv_obj_t *label = lv_label_create(padre);
    lv_label_set_text(label, texto);
    lv_obj_set_style_text_font(label, tam >= 24 ? &lv_font_montserrat_24 : &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    return label;
}

static lv_obj_t *crear_chip(lv_obj_t *padre, const char *titulo, lv_obj_t **valor)
{
    lv_obj_t *chip = lv_obj_create(padre);
    lv_obj_remove_style_all(chip);
    lv_obj_set_size(chip, 102, 54);
    lv_obj_set_style_bg_color(chip, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(chip, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(chip, 8, 0);
    lv_obj_set_style_border_width(chip, 1, 0);
    lv_obj_set_style_border_color(chip, lv_color_hex(NERA_COLOR_BORDE), 0);
    lv_obj_set_style_pad_all(chip, 8, 0);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *label_titulo = crear_label(chip, titulo, 14, 0x64748B);
    lv_obj_set_style_text_font(label_titulo, &lv_font_montserrat_12, 0);

    *valor = crear_label(chip, "--", 14, NERA_COLOR_TEXTO);
    lv_obj_set_style_text_font(*valor, &lv_font_montserrat_18, 0);
    return chip;
}

esp_err_t nera_pantalla_principal_mostrar(void)
{
    ESP_RETURN_ON_FALSE(lvgl_port_lock(0), ESP_ERR_TIMEOUT, "nera_ui", "Timeout tomando lock LVGL");

    pantalla = lv_obj_create(NULL);
    lv_obj_remove_style_all(pantalla);
    lv_obj_set_style_bg_color(pantalla, lv_color_hex(NERA_COLOR_FONDO), 0);
    lv_obj_set_style_bg_opa(pantalla, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(pantalla, 14, 0);

    lv_obj_t *barra_superior = lv_obj_create(pantalla);
    lv_obj_remove_style_all(barra_superior);
    lv_obj_set_size(barra_superior, LV_PCT(100), 28);
    lv_obj_align(barra_superior, LV_ALIGN_TOP_MID, 0, 0);

    label_hora = crear_label(barra_superior, "NERA 12:00", 14, NERA_COLOR_TEXTO);
    lv_obj_align(label_hora, LV_ALIGN_LEFT_MID, 0, 0);

    label_ble = crear_label(barra_superior, "BLE sin conexion", 14, 0x64748B);
    lv_obj_set_style_text_font(label_ble, &lv_font_montserrat_12, 0);
    lv_obj_align(label_ble, LV_ALIGN_RIGHT_MID, 0, 0);

    arco_bienestar = lv_arc_create(pantalla);
    lv_obj_set_size(arco_bienestar, 174, 174);
    lv_obj_align(arco_bienestar, LV_ALIGN_TOP_MID, 0, 42);
    lv_arc_set_range(arco_bienestar, 0, 100);
    lv_arc_set_bg_angles(arco_bienestar, 135, 45);
    lv_obj_remove_style(arco_bienestar, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arco_bienestar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(arco_bienestar, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arco_bienestar, lv_color_hex(NERA_COLOR_BORDE), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arco_bienestar, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arco_bienestar, lv_color_hex(NERA_COLOR_EXITO), LV_PART_INDICATOR);

    label_bienestar = crear_label(pantalla, "100", 24, NERA_COLOR_TEXTO);
    lv_obj_set_style_text_font(label_bienestar, &lv_font_montserrat_32, 0);
    lv_obj_align_to(label_bienestar, arco_bienestar, LV_ALIGN_CENTER, 0, -8);

    label_estado = crear_label(pantalla, "Bienestar estable", 14, NERA_COLOR_PRIMARIO);
    lv_obj_align_to(label_estado, arco_bienestar, LV_ALIGN_CENTER, 0, 28);

    lv_obj_t *contenedor_metricas = lv_obj_create(pantalla);
    lv_obj_remove_style_all(contenedor_metricas);
    lv_obj_set_size(contenedor_metricas, LV_PCT(100), 124);
    lv_obj_align(contenedor_metricas, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(contenedor_metricas, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(contenedor_metricas, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(contenedor_metricas, 10, 0);

    crear_chip(contenedor_metricas, "HR", &label_hr);
    crear_chip(contenedor_metricas, "SpO2", &label_spo2);
    crear_chip(contenedor_metricas, "Temp", &label_temp);
    crear_chip(contenedor_metricas, "Mov", &label_mov);

    lv_screen_load(pantalla);

    nera_metricas_reloj_t inicial = {
        .hr = 76,
        .spo2 = 98,
        .temp = 36.6f,
        .ax = 0.02f,
        .ay = 0.01f,
        .az = 1.0f,
        .ble_conectado = false,
    };
    nera_pantalla_principal_actualizar(&inicial);

    lvgl_port_unlock();
    return ESP_OK;
}

void nera_pantalla_principal_actualizar(const nera_metricas_reloj_t *metricas)
{
    if (metricas == NULL || pantalla == NULL) {
        return;
    }

    char texto[32];
    const int estres = calcular_estres_estimado(metricas);
    const int bienestar = 100 - estres;
    const lv_color_t color = color_estres(estres);

    snprintf(texto, sizeof(texto), "%d", bienestar);
    lv_label_set_text(label_bienestar, texto);
    lv_arc_set_value(arco_bienestar, bienestar);
    lv_obj_set_style_arc_color(arco_bienestar, color, LV_PART_INDICATOR);

    if (estres < 35) {
        lv_label_set_text(label_estado, "Bienestar estable");
        lv_obj_set_style_text_color(label_estado, lv_color_hex(NERA_COLOR_EXITO), 0);
    } else if (estres <= 65) {
        lv_label_set_text(label_estado, "Estres moderado");
        lv_obj_set_style_text_color(label_estado, lv_color_hex(NERA_COLOR_AMARILLO), 0);
    } else {
        lv_label_set_text(label_estado, "Alerta de estres");
        lv_obj_set_style_text_color(label_estado, lv_color_hex(NERA_COLOR_PELIGRO), 0);
    }

    lv_label_set_text(label_ble, metricas->ble_conectado ? "BLE conectado" : "BLE sin conexion");
    lv_obj_set_style_text_color(label_ble,
                                metricas->ble_conectado ? lv_color_hex(NERA_COLOR_SECUNDARIO) : lv_color_hex(0x64748B),
                                0);

    snprintf(texto, sizeof(texto), "%d bpm", metricas->hr);
    lv_label_set_text(label_hr, texto);

    snprintf(texto, sizeof(texto), "%d%%", metricas->spo2);
    lv_label_set_text(label_spo2, texto);

    snprintf(texto, sizeof(texto), "%.1f C", (double)metricas->temp);
    lv_label_set_text(label_temp, texto);

    snprintf(texto, sizeof(texto), "%.2fg", (double)(fabsf(metricas->ax) + fabsf(metricas->ay) + fabsf(metricas->az - 1.0f)));
    lv_label_set_text(label_mov, texto);
}
