/**
 * @file app_main.cpp
 * @brief Punto de entrada del firmware NERA.
 *
 * En esta Etapa 1B:
 * - Se inicializa el sistema base (NVS, AppState, EventBus).
 * - Se inicializa el PWM de Backlight (LEDC) y se enciende la pantalla.
 * - Se inicializa el bus SPI con DMA y el controlador ST7789T3 (240x320 px).
 * - Se dibuja el patrón de prueba visual de NERA para verificar hardware físico.
 */

#include <stdio.h>
#include <string.h>

// ESP-IDF
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// NERA — Módulos propios
#include "nera_config.h"
#include "utils/logger.h"
#include "core/app_state.h"
#include "core/event_bus.h"
#include "display/display_driver.h"
#include "display/display_manager.h"
#include "lvgl/lvgl_task.h"
#include "sensors/heart_rate/heart_rate_filter.h"
#include "sensors/heart_rate/mock_heart_rate_sensor.h"
#include "sensors/temperature/mock_temperature_sensor.h"

static const char *TAG = NERA_TAG_MAIN;
static MockHeartRateSensor s_heart_rate_sensor;
static HeartRateFilter s_heart_rate_filter;
static MockTemperatureSensor s_temperature_sensor;

static void battery_mock_task(void *arg)
{
    (void)arg;
    uint8_t percentage = 100;

    while (true) {
        NeraBatteryData battery = {};
        battery.percentage = percentage;
        battery.voltage_mv = 4200.0f - ((100.0f - percentage) * 12.0f);
        battery.sensor_ok = true;
        battery.state = percentage <= 5 ? NERA_BATTERY_CRITICAL :
                        percentage <= 20 ? NERA_BATTERY_LOW :
                        percentage >= 100 ? NERA_BATTERY_FULL :
                        NERA_BATTERY_DISCHARGING;
        app_state_set_battery(&battery);

        if (percentage > 10) {
            percentage--;
        } else {
            percentage = 100;
        }
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

// =============================================================================
// FUNCIONES PRIVADAS
// =============================================================================

/**
 * @brief Imprime información del hardware en el log de debug.
 */
static void log_system_info(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    NERA_LOGI(TAG, "============================================");
    NERA_LOGI(TAG, "  NERA Smartwatch Firmware v%s", NERA_FW_VERSION_STR);
    NERA_LOGI(TAG, "============================================");
    NERA_LOGI(TAG, "Chip: ESP32-S3, %d cores @ hasta 240 MHz", chip_info.cores);
    NERA_LOGI(TAG, "Revision: %d", chip_info.revision);

    size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t free_dma      = heap_caps_get_free_size(MALLOC_CAP_DMA);
    NERA_LOGI(TAG, "RAM interna libre: %zu KB", free_internal / 1024);
    NERA_LOGI(TAG, "RAM DMA libre:     %zu KB", free_dma / 1024);

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    if (free_psram > 0) {
        NERA_LOGI(TAG, "PSRAM libre:       %zu KB (%zu MB)",
                  free_psram / 1024, free_psram / 1024 / 1024);
    } else {
        NERA_LOGW(TAG, "PSRAM: no detectada o no habilitada");
    }

    NERA_LOGI(TAG, "Pantalla: %dx%d px (ST7789T3, SPI)", NERA_LCD_WIDTH, NERA_LCD_HEIGHT);
    NERA_LOGI(TAG, "============================================");
}

/**
 * @brief Inicializa NVS (Non-Volatile Storage).
 */
static esp_err_t init_nvs(void) {
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || 
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        NERA_LOGW(TAG, "NVS: partición corrupta o versión incompatible. Borrando...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        NERA_LOGE(TAG, "Error al inicializar NVS: %s", esp_err_to_name(err));
        return err;
    }

    NERA_LOGI(TAG, "NVS inicializado correctamente");
    return ESP_OK;
}

/**
 * @brief Tarea de monitoreo periódico de memoria.
 */
static void memory_monitor_task(void *arg) {
    const char *MTAG = "NERA/MEM";

    while (true) {
        size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        size_t min_internal  = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        size_t free_psram    = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

        ESP_LOGI(MTAG, "RAM libre: %zu KB (mín: %zu KB) | PSRAM: %zu KB",
                 free_internal / 1024,
                 min_internal / 1024,
                 free_psram / 1024);

        vTaskDelay(pdMS_TO_TICKS(30000));
    }

}

static void heart_rate_task(void *arg)
{
    (void)arg;

    if (s_heart_rate_sensor.begin() != ESP_OK) {
        NERA_LOGE(TAG, "No se pudo iniciar el sensor mock de frecuencia cardíaca");
        vTaskDelete(NULL);
        return;
    }

    s_heart_rate_filter.reset();

    while (true) {
        if (s_heart_rate_sensor.update() != ESP_OK) {
            NERA_LOGW(TAG, "No se pudo actualizar frecuencia cardíaca");
        } else {
            float filtered_bpm = 0.0f;
            if (!s_heart_rate_filter.add_sample(s_heart_rate_sensor.get_bpm(),
                                                &filtered_bpm)) {
                NERA_LOGW(TAG, "Muestra de frecuencia cardíaca descartada");
                vTaskDelay(pdMS_TO_TICKS(NERA_SENSOR_UPDATE_INTERVAL_MS));
                continue;
            }

            NeraHeartData heart = {};
            heart.bpm = filtered_bpm;
            heart.bpm_min = heart.bpm;
            heart.bpm_max = heart.bpm;
            heart.bpm_avg = heart.bpm;
            heart.is_valid = s_heart_rate_sensor.is_valid();
            heart.sensor_ok = true;
            heart.last_update_us = esp_timer_get_time();
            app_state_set_heart(&heart);
        }
        vTaskDelay(pdMS_TO_TICKS(NERA_SENSOR_UPDATE_INTERVAL_MS));
    }
}

static void temperature_task(void *arg)
{
    (void)arg;

    if (s_temperature_sensor.begin() != ESP_OK) {
        NERA_LOGE(TAG, "No se pudo iniciar el sensor mock de temperatura");
        vTaskDelete(NULL);
        return;
    }

    while (true) {
        if (s_temperature_sensor.update() != ESP_OK) {
            NERA_LOGW(TAG, "No se pudo actualizar temperatura");
        } else {
            NeraTempData temperature = {};
            temperature.celsius = s_temperature_sensor.get_temperature_celsius();
            temperature.celsius_min = temperature.celsius;
            temperature.celsius_max = temperature.celsius;
            temperature.is_valid = s_temperature_sensor.is_valid();
            temperature.sensor_ok = true;
            temperature.last_update_us = esp_timer_get_time();
            app_state_set_temp(&temperature);
        }
        vTaskDelay(pdMS_TO_TICKS(NERA_SENSOR_UPDATE_INTERVAL_MS));
    }
}

// =============================================================================
// PUNTO DE ENTRADA PRINCIPAL
// =============================================================================

extern "C" void app_main(void) {
    // Pausa breve para estabilización de USB CDC
    vTaskDelay(pdMS_TO_TICKS(100));

    // --- Paso 1: Diagnóstico de hardware ---
    log_system_info();

    // --- Paso 2: NVS ---
    NERA_LOGI(TAG, "[1/5] Inicializando NVS...");
    if (init_nvs() != ESP_OK) {
        NERA_LOGE(TAG, "Error crítico en NVS.");
        return;
    }

    // --- Paso 3: Estado global ---
    NERA_LOGI(TAG, "[2/5] Inicializando estado global...");
    if (app_state_init() != ESP_OK) {
        NERA_LOGE(TAG, "Error crítico en estado global.");
        return;
    }

    // --- Paso 4: Event Bus ---
    NERA_LOGI(TAG, "[3/5] Inicializando event bus...");
    if (event_bus_init() != ESP_OK) {
        NERA_LOGE(TAG, "Error crítico en event bus.");
        return;
    }

    // --- Paso 5: Hardware de Pantalla LCD (ST7789T3) ---
    NERA_LOGI(TAG, "[4/5] Inicializando hardware de pantalla LCD (ST7789T3)...");
    esp_err_t ret = display_driver_init();
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Error inicializando driver de pantalla: %s", esp_err_to_name(ret));
        return;
    }

    // --- Paso 6: Backlight PWM ---
    NERA_LOGI(TAG, "[5/5] Inicializando Backlight PWM (LEDC)...");
    ret = display_manager_init();
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Error inicializando backlight: %s", esp_err_to_name(ret));
        return;
    }

    // El patrón de colores se reserva para el diagnóstico del hardware.
    // El arranque normal debe quedar bajo el control de LVGL.
    NERA_LOGI(TAG, "Preparando pantalla para la interfaz LVGL...");
    ret = display_driver_fill_screen(0x0000);
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "No se pudo limpiar la pantalla: %s", esp_err_to_name(ret));
        return;
    }

    // Actualizar modo a READY
    app_state_set_mode(NERA_MODE_READY);

    NERA_LOGI(TAG, "");
    NERA_LOGI(TAG, "+----------------------------------------+");
    NERA_LOGI(TAG, "| NERA Firmware - Etapa 1B               |");
    NERA_LOGI(TAG, "| Driver LCD + Backlight PWM OK          |");
    NERA_LOGI(TAG, "| Display 240x320 activo con test visual |");
    NERA_LOGI(TAG, "|                                        |");
    NERA_LOGI(TAG, "| Siguiente paso: Etapa 1C               |");
    NERA_LOGI(TAG, "| Integracion motor grafico LVGL v8      |");
    NERA_LOGI(TAG, "+----------------------------------------+");
    NERA_LOGI(TAG, "");

    // --- Paso 8: Crear tarea LVGL para UI ---
    NERA_LOGI(TAG, "[6/6] Lanzando motor gráfico LVGL v8...");
    xTaskCreate(
        lvgl_task,
        "nera_lvgl_ui",
        NERA_UI_TASK_STACK_SIZE,
        NULL,
        NERA_UI_TASK_PRIORITY,
        NULL
    );

    NERA_LOGI(TAG, "");
    NERA_LOGI(TAG, "+----------------------------------------+");
    NERA_LOGI(TAG, "| NERA Firmware - Etapa 1D               |");
    NERA_LOGI(TAG, "| Watchface + pantalla Salud             |");
    NERA_LOGI(TAG, "| Navegacion touch/swipe basica          |");
    NERA_LOGI(TAG, "| Datos mock integrados con AppState     |");
    NERA_LOGI(TAG, "|                                        |");
    NERA_LOGI(TAG, "| Siguiente paso: Etapa 1E               |");
    NERA_LOGI(TAG, "| Refinar touch/gestos y componentes UI  |");
    NERA_LOGI(TAG, "+----------------------------------------+");
    NERA_LOGI(TAG, "");

    // Monitoreo de memoria en background
    xTaskCreate(memory_monitor_task, "nera_mem_mon", 2048, NULL, 1, NULL);
    xTaskCreate(heart_rate_task, "nera_hr_mock", NERA_SENSOR_TASK_STACK_SIZE, NULL,
                NERA_SENSOR_TASK_PRIORITY, NULL);
    xTaskCreate(temperature_task, "nera_temp_mock", NERA_SENSOR_TASK_STACK_SIZE, NULL,
                NERA_SENSOR_TASK_PRIORITY, NULL);
    xTaskCreate(battery_mock_task, "nera_battery_mock", NERA_SENSOR_TASK_STACK_SIZE, NULL,
                NERA_SENSOR_TASK_PRIORITY, NULL);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
