#include "gpio5_signal_test.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "NERA/GPIO5_TEST";

// MODIFICABLE: parametros de la prueba de senal SIN bobinas.
#define TEST_GPIO GPIO_NUM_5
#define TEST_FREQUENCY_HZ 15U
#define TEST_ACTIVE_US 2500U
#define TEST_RESOLUTION_BITS 14U
#define TEST_PERIOD_TICKS (1U << TEST_RESOLUTION_BITS)
// Redondear al tick LEDC mas cercano: 614/16384 = 3,7476 % bajo.
#define TEST_ACTIVE_TICKS ((uint32_t)(((uint64_t)TEST_PERIOD_TICKS * \
    TEST_FREQUENCY_HZ * TEST_ACTIVE_US + 500000ULL) / 1000000ULL))

_Static_assert(TEST_ACTIVE_TICKS > 0 && TEST_ACTIVE_TICKS < TEST_PERIOD_TICKS,
               "El pulso debe ser menor que el periodo PWM");

esp_err_t gpio5_signal_test_start(void)
{
    // Precargar alto antes de habilitar la salida: alto apaga el driver.
    // Esto NO controla el estado del circuito durante ROM/bootloader/reset.
    ESP_RETURN_ON_ERROR(gpio_set_level(TEST_GPIO, 1), TAG, "Nivel inicial");
    const gpio_config_t gpio = {
        .pin_bit_mask = 1ULL << TEST_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&gpio), TAG, "Configurar GPIO5");

    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = TEST_FREQUENCY_HZ,
        .clk_cfg = LEDC_USE_XTAL_CLK,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), TAG, "Configurar timer LEDC");

    const ledc_channel_config_t channel = {
        .gpio_num = TEST_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = TEST_ACTIVE_TICKS,
        .hpoint = 0,
        .flags.output_invert = 1,
    };
    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel), TAG, "Configurar PWM GPIO5");

    ESP_LOGW(TAG, "PRUEBA CONTINUA SIN BOBINAS: GPIO5, %lu Hz, activo BAJO ~2.5 ms, alto ~64.17 ms",
             (unsigned long)ledc_get_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0));
    ESP_LOGW(TAG, "PWM independiente del reloj y de las alertas ESP-NOW; no conectar bobinas");
    return ESP_OK;
}
