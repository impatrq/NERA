/**
 * @file lvgl_port.h
 * @brief Puente de integración entre LVGL v8 y el driver display_driver.
 *
 * Este módulo proporciona los callbacks necesarios para que LVGL se comunique
 * con el hardware de la pantalla (ST7789T3) a través del módulo display_driver.
 *
 * CONCEPTO: Callback
 * Un callback es una función que le pasamos a otra librería para que la llame
 * cuando necesite algo. Por ejemplo:
 *   LVGL dice: "cuando tengas píxeles para mostrar, llama a mi_funcion_flush"
 *   Nosotros le pasamos: display_flush_callback
 *   LVGL la llama automáticamente cuando necesita enviar datos a pantalla.
 *
 * CONCEPTO: Framebuffer / Buffer de dibujo
 * Es la memoria donde LVGL dibuja. Tiene dos buffers:
 *   Buffer 1: LVGL dibuja aquí
 *   Buffer 2: LVGL envía este buffer a la pantalla mientras dibuja en el otro
 * Esto se llama "double buffering" y evita parpadeos.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el puerto LVGL.
 *
 * Configura:
 * - Los buffers de dibujo en PSRAM
 * - El driver de pantalla de LVGL
 * - Los callbacks de flush y tick timer
 *
 * Debe llamarse ANTES de lv_init().
 *
 * @return ESP_OK si se inicializó correctamente.
 */
esp_err_t lvgl_port_init(void);

/**
 * @brief Callback de "flush" (vaciado) de LVGL.
 *
 * LVGL llama a esta función cuando tiene una región de píxeles lista para
 * ser enviada a la pantalla. Nosotros usamos display_driver_draw_bitmap
 * para hacerlo.
 *
 * @param drv Puntero al driver de LVGL (uso interno, lo genera LVGL).
 * @param area Área rectangular con píxeles listos (x1, y1, x2, y2).
 * @param color_map Buffer de píxeles en formato RGB565 a enviar a pantalla.
 */
void lvgl_port_flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

/**
 * @brief Callback de tick timer para LVGL.
 *
 * FreeRTOS llama a esta función cada 5 ms para que LVGL actualice
 * animaciones, detecte toques, etc.
 *
 * @param timer Temporizador que invoca el callback.
 */
void lvgl_port_tick_callback(TimerHandle_t timer);

#ifdef __cplusplus
}
#endif
