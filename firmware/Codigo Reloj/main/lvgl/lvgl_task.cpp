/** Tarea grafica: inicializa LVGL, registra tactil y procesa la pantalla. */
#include "lvgl_task.h"
#include "lvgl_port.h"
#include "nera_config.h"
#include "display/display_manager.h"
#include "ui/ui_manager.h"
#include "touch/touch_driver.h"
#include "connectivity/ble_manager.h"
#include "services/history_service.h"
#include "utils/logger.h"
#include "lvgl.h"
#if CONFIG_NERA_ESPNOW_DIAGNOSTIC
#include "connectivity/espnow_diagnostic.h"
#endif

void lvgl_task(void *) {
    if (lvgl_port_init()!=ESP_OK) {
        ESP_LOGE("NERA/LVGL", "No se pudo iniciar el motor grafico");
        vTaskDelete(nullptr);
        return;
    }
    const esp_err_t touch=touch_driver_init();
    if (touch==ESP_OK) {
        static lv_indev_drv_t input;
        lv_indev_drv_init(&input);
        input.type=LV_INDEV_TYPE_POINTER;
        input.read_cb=touch_driver_read;
        lv_indev_drv_register(&input);
    } else ESP_LOGW("NERA/TOUCH", "Tactil no disponible: %s",esp_err_to_name(touch));
    ESP_ERROR_CHECK(ui_manager_init());
    ESP_LOGI("NERA/LVGL", "Pantalla lista");
#if CONFIG_NERA_ESPNOW_DIAGNOSTIC
    // IMPORTANTE: primero reservar objetos/buffers y terminar el primer dibujo.
    // La radio no modifica LVGL y un fallo suyo no detiene la pantalla.
    lv_timer_handler();
    const esp_err_t radio=espnow_diagnostic_init();
    if (radio!=ESP_OK)
        ESP_LOGW("NERA/RADIO","No se pudo iniciar ESP-NOW: %s; LCD continua",esp_err_to_name(radio));
#endif
    const esp_err_t history = history_service_init();
    if (history == ESP_OK) {
        if (xTaskCreate(history_service_task, "nera_history", 4096, nullptr, 2, nullptr) != pdPASS)
            ESP_LOGE("NERA/HISTORY", "No se pudo crear la tarea de historial");
    } else {
        ESP_LOGW("NERA/HISTORY", "Historial deshabilitado: %s", esp_err_to_name(history));
    }
    // Iniciar BLE despues de reservar la pantalla; publica solo snapshots reales.
    const esp_err_t ble = ble_manager_init();
    if (ble == ESP_OK) {
        if (xTaskCreate(ble_manager_task, "nera_ble_data", 4096, nullptr, 2, nullptr) != pdPASS)
            ESP_LOGE("NERA/BLE", "No se pudo crear la tarea de datos BLE");
    } else {
        ESP_LOGW("NERA/BLE", "No se pudo iniciar BLE: %s", esp_err_to_name(ble));
    }
    for (;;) {
        uint32_t delay=lv_timer_handler();
        // Solo el toque reinicia el minuto. Las lecturas se siguen actualizando
        // aunque la retroiluminacion este apagada.
        const bool awake = lv_disp_get_inactive_time(lv_disp_get_default()) < CONFIG_NERA_SCREEN_TIMEOUT_MS;
        const esp_err_t screen = display_manager_set_screen_awake(awake);
        if (screen != ESP_OK)
            ESP_LOGW("NERA/LVGL", "No se pudo cambiar backlight: %s", esp_err_to_name(screen));
        if (delay<1) delay=1;
        if (delay>20) delay=20;
        const TickType_t ticks=pdMS_TO_TICKS(delay);
        vTaskDelay(ticks ? ticks : 1);
    }
}
