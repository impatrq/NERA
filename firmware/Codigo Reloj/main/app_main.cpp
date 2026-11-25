/** Arranque del ESP32-S3: LCD, brillo y tarea grafica. */
#include "nera_config.h"
#include "display/display_driver.h"
#include "display/display_manager.h"
#include "lvgl/lvgl_task.h"
#include "sensor_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

extern "C" void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI("NERA/MAIN", "NERA: LCD, tactil y sensores MAX30102/MPU6050/MLX90614");
    ESP_ERROR_CHECK(display_driver_init());
    ESP_ERROR_CHECK(display_manager_init());
    ESP_ERROR_CHECK(display_driver_fill_screen(0x0000));
    // FUNCIONAL: sensores inicializa I2C1 mediante drivers/i2c_bus.c.
    // Su fallo no impide usar pantalla y tactil; la UI mostrara "Sin lectura".
    const esp_err_t sensor_start = sensor_service_start();
    if (sensor_start != ESP_OK)
        ESP_LOGE("NERA/MAIN", "No se pudo iniciar sensores: %s", esp_err_to_name(sensor_start));
    // FUNCIONAL: todo acceso a objetos LVGL pertenece a esta tarea.
    if (xTaskCreate(lvgl_task, "nera_display", NERA_UI_TASK_STACK_SIZE, nullptr,
                    NERA_UI_TASK_PRIORITY, nullptr)!=pdPASS)
        ESP_LOGE("NERA/MAIN", "No se pudo iniciar LVGL: memoria insuficiente");
}
