/**
 * @file display_manager.cpp
 * @brief Implementación del control de brillo por hardware PWM (LEDC).
 *
 * ¿Qué es PWM (Pulse Width Modulation) y el periférico LEDC?
 *
 * Los LEDs del Backlight de la pantalla necesitan variar su brillo. En lugar de
 * variar el voltaje de forma analógica (lo cual es ineficiente y genera calor),
 * se envía una señal cuadrada rápida (5000 veces por segundo, 5 kHz).
 *
 * - Si la señal está en HIGH el 50% del tiempo y en LOW el 50% restante (Duty Cycle 50%),
 *   el ojo humano percibe la mitad del brillo.
 * - Si está en HIGH el 100% del tiempo (Duty Cycle 255), el brillo es máximo.
 *
 * El periférico LEDC (LED Controller) del ESP32-S3 maneja esto 100% por hardware
 * sin consumir ciclos de CPU.
 */

#include "display_manager.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "nera_config.h"
#include "utils/logger.h"

static const char *TAG = NERA_TAG_POWER;

static uint8_t s_current_brightness = 0;
static bool s_is_initialized = false;

esp_err_t display_manager_init(void) {
    if (s_is_initialized) return ESP_OK;

    NERA_LOGI(TAG, "Configurando PWM de Backlight en GPIO %d (LEDC Canal %d, %d Hz)...",
              NERA_LCD_PIN_BL, NERA_BL_LEDC_CHANNEL, NERA_BL_PWM_FREQ_HZ);

    // 1. Configurar el Timer del periférico LEDC
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num        = NERA_BL_LEDC_TIMER;
    ledc_timer.duty_resolution  = NERA_BL_PWM_RESOLUTION; // 8 bits (0 a 255)
    ledc_timer.freq_hz          = NERA_BL_PWM_FREQ_HZ;    // 5000 Hz
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Error configurando timer LEDC: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Configurar el Canal LEDC asignado al GPIO del Backlight
    ledc_channel_config_t ledc_channel = {};
    ledc_channel.speed_mode     = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel        = NERA_BL_LEDC_CHANNEL;
    ledc_channel.timer_sel      = NERA_BL_LEDC_TIMER;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num       = NERA_LCD_PIN_BL;
    ledc_channel.duty           = 0; // Inicia apagado para evitar destellos
    ledc_channel.hpoint         = 0;

    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Error configurando canal LEDC: %s", esp_err_to_name(ret));
        return ret;
    }

    // 3. Instalar el servicio de fade por hardware
    ledc_fade_func_install(0);

    s_is_initialized = true;

    // Encender la pantalla con el brillo por defecto (ej: 200 = ~80%)
    display_manager_set_brightness(NERA_BL_DEFAULT);

    NERA_LOGI(TAG, "Backlight LCD inicializado con exito (Brillo: %d/255)", NERA_BL_DEFAULT);
    return ESP_OK;
}

esp_err_t display_manager_set_brightness(uint8_t brightness) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;

    s_current_brightness = brightness;

    esp_err_t ret = ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, NERA_BL_LEDC_CHANNEL, brightness, 0);
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Error actualizando duty cycle de brillo: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

uint8_t display_manager_get_brightness(void) {
    return s_current_brightness;
}

esp_err_t display_manager_fade_brightness(uint8_t target_brightness, uint32_t duration_ms) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;

    s_current_brightness = target_brightness;

    esp_err_t ret = ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, NERA_BL_LEDC_CHANNEL, target_brightness, duration_ms);
    if (ret == ESP_OK) {
        ledc_fade_start(LEDC_LOW_SPEED_MODE, NERA_BL_LEDC_CHANNEL, LEDC_FADE_NO_WAIT);
    }
    return ret;
}
