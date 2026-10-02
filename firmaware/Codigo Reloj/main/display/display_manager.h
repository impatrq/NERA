/**
 * @file display_manager.h
 * @brief Gestor de alto nivel de la pantalla (Brillo PWM, energía y timeout).
 *
 * Controla la retroiluminación (Backlight) mediante PWM por hardware (LEDC)
 * permitiendo regular el brillo suavemente (0 a 255) y preparar la pantalla
 * para estados de ahorro de energía (Atenuado / Apagado).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el periférico PWM (LEDC) para el control del backlight.
 *
 * Configura:
 * - Timer LEDC a 5 kHz (resolución de 8 bits: 0 a 255).
 * - Canal LEDC conectado al GPIO 46 de la pantalla Waveshare.
 * - Enciende la pantalla con el brillo por defecto (80%).
 *
 * @return ESP_OK si la configuración fue correcta.
 */
esp_err_t display_manager_init(void);

/**
 * @brief Establece el brillo de la pantalla.
 *
 * @param brightness Nivel de brillo de 0 (apagado) a 255 (100% brillo máximo).
 * @return ESP_OK si se aplicó el ciclo de trabajo PWM.
 */
esp_err_t display_manager_set_brightness(uint8_t brightness);

/**
 * @brief Obtiene el nivel de brillo actual.
 *
 * @return Nivel de brillo actual (0 - 255).
 */
uint8_t display_manager_get_brightness(void);

/**
 * @brief Transición gradual de brillo (fade suave).
 *
 * Cambia suavemente el brillo desde el valor actual hasta el objetivo,
 * ofreciendo una sensación visual moderna de smartwatch.
 *
 * @param target_brightness Brillo destino (0 - 255).
 * @param duration_ms Duración de la transición en milisegundos.
 * @return ESP_OK si la transición fue programada.
 */
esp_err_t display_manager_fade_brightness(uint8_t target_brightness, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif
