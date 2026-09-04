/**
 * @file display_driver.h
 * @brief Driver de bajo nivel para la pantalla LCD Waveshare 2" (ST7789T3 vía SPI).
 *
 * Este módulo se encarga de la comunicación directa con el chip controlador
 * ST7789T3 que gobierna los píxeles de la pantalla LCD.
 *
 * Utiliza el subsistema oficial `esp_lcd` de ESP-IDF junto con transferencias
 * DMA (Direct Memory Access) para enviar datos a la pantalla sin sobrecargar
 * el procesador.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "nera_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FUNCIONES PÚBLICAS DEL DRIVER
// =============================================================================

/**
 * @brief Inicializa el bus SPI, el GPIO de reset y el controlador ST7789T3.
 *
 * Configura:
 * - Bus SPI2 con DMA habilitado a 40 MHz.
 * - Secuencia de inicialización del ST7789T3 (orientación, inversión de color IPS).
 * - Enciende el display.
 *
 * @return ESP_OK si la inicialización fue exitosa, o código de error.
 */
esp_err_t display_driver_init(void);

/**
 * @brief Obtiene el handle del panel LCD de ESP-IDF.
 * 
 * Necesario más adelante para vincular el driver directamente con LVGL.
 *
 * @return Handle del panel LCD (esp_lcd_panel_handle_t).
 */
esp_lcd_panel_handle_t display_driver_get_panel_handle(void);

/**
 * @brief Dibuja un bloque de píxeles (bitmap) en un área rectangular de la pantalla.
 *
 * @param x_start Coordenada X inicial (0 a NERA_LCD_WIDTH - 1).
 * @param y_start Coordenada Y inicial (0 a NERA_LCD_HEIGHT - 1).
 * @param x_end   Coordenada X final (inclusiva).
 * @param y_end   Coordenada Y final (inclusiva).
 * @param color_data Puntero al buffer de píxeles en formato RGB565 (16 bits por pixel).
 *
 * @return ESP_OK si la transferencia fue enviada correctamente al bus SPI.
 */
esp_err_t display_driver_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data);

/**
 * @brief Llena toda la pantalla con un color uniforme en formato RGB565.
 *
 * @param rgb565 Color de 16 bits (formato RRRRRGGGGGGBBBBB).
 * @return ESP_OK si se completó la operación.
 */
esp_err_t display_driver_fill_screen(uint16_t rgb565);

/**
 * @brief Dibuja un patrón de prueba de calibración y color en la pantalla.
 *
 * Dibuja un degradado y bloques de color semánticos de NERA (Cian, Verde, Ámbar, Rojo)
 * para comprobar visualmente la orientación, contraste y fidelidad de los colores.
 *
 * @return ESP_OK si se dibujó correctamente.
 */
esp_err_t display_driver_draw_test_pattern(void);

#ifdef __cplusplus
}
#endif
