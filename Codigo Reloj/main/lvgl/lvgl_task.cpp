/**
 * @file lvgl_task.cpp
 * @brief Implementación de la tarea FreeRTOS para LVGL.
 *
 * Esta es la tarea responsable de:
 * 1. Ejecutar el bucle principal de eventos de LVGL (lv_task_handler)
 * 2. Procesar animaciones y interacciones
 * 3. Refrescar la pantalla
 *
 * Ejecuta a baja prioridad (5) con stack de 8 KB.
 */

#include "lvgl_task.h"
#include "lvgl_port.h"
#include "nera_config.h"
#include "ui/ui_manager.h"
#include "utils/logger.h"
#include "lvgl.h"
#include "touch/touch_driver.h"

static const char *TAG = NERA_TAG_LVGL;

// =============================================================================
// FUNCIÓN PÚBLICA: Tarea LVGL
// =============================================================================

void lvgl_task(void *pvParameters)
{
    NERA_LOGI(TAG, "LVGL task started (stack: %d bytes, priority: %d)",
              NERA_UI_TASK_STACK_SIZE, NERA_UI_TASK_PRIORITY);

    // =========================================================================
    // FASE 1: Inicializar el puerto LVGL
    // =========================================================================

    /**
     * lvgl_port_init() hace toda la configuración pesada:
     * - Asigna buffers en PSRAM
     * - Inicializa LVGL
     * - Configura el driver de pantalla
     * - Inicia el timer de ticks
     *
     * Si esto falla, el smartwatch no puede mostrar interfaz.
     */
    esp_err_t ret = lvgl_port_init();
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Failed to initialize LVGL port: 0x%x", ret);
        // No hay recuperación posible. La tarea se suspenderá.
        vTaskSuspend(NULL);
        return;
    }

    ret = touch_driver_init();
    if (ret != ESP_OK) {
        NERA_LOGW(TAG, "Touch no disponible; UI continuará sin navegación táctil");
    }
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_driver_read;
    lv_indev_drv_register(&indev_drv);

    // =========================================================================
    // FASE 2: Crear la interfaz inicial
    // =========================================================================

    ret = ui_manager_init();
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Failed to initialize UI manager: 0x%x", ret);
        vTaskSuspend(NULL);
        return;
    }

    NERA_LOGI(TAG, "UI initialization complete. Starting event loop...");

    // =========================================================================
    // FASE 3: Bucle principal de LVGL
    // =========================================================================

    /**
     * CONCEPTO: Bucle de eventos
     * Un bucle de eventos es el corazón de cualquier interfaz gráfica:
     *
     *   while (running) {
     *       process_events();     // ¿Tocó el usuario?
     *       animate_widgets();    // Animar transiciones
     *       redraw_changed();     // Redibujar lo que cambió
     *       sleep_briefly();      // No consumir 100% CPU
     *   }
     *
     * lv_task_handler() hace todo esto.
     *
     * Parámetro de lv_task_handler:
     * Retorna la cantidad de milisegundos hasta que el siguiente evento
     * sucederá. Nosotros dormimos ese tiempo para ahorrar energía.
     */
    while (1) {
        // Procesar eventos de LVGL
        uint32_t delay_ms = lv_task_handler();

        // Evitar que el delay sea demasiado corto (mínimo 1 ms)
        if (delay_ms < 1) {
            delay_ms = 1;
        }

        // Dormir sin bloquear otras tareas de FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }

    // Este punto nunca se alcanza, pero lo dejamos para claridad de código.
    vTaskSuspend(NULL);
}
