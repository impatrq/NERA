/**
 * @file lvgl_port.cpp
 * @brief Implementación del puente LVGL ↔ driver display.
 *
 * IMPORTANTE: Este módulo es crítico porque es donde LVGL se conecta con
 * el hardware real de la pantalla. Los errores aquí causarán parpadeos,
 * corrupción de imagen o pantalla en blanco.
 */

#include "lvgl_port.h"
#include "display/display_driver.h"
#include "nera_config.h"
#include "utils/logger.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static const char *TAG = NERA_TAG_LVGL;

// =============================================================================
// VARIABLES GLOBALES — Buffers de dibujo LVGL
// =============================================================================

/**
 * Dos buffers de dibujo en PSRAM:
 * - El primero es donde LVGL dibuja
 * - El segundo se envía a la pantalla mientras LVGL dibuja en el primero
 *
 * Tamaño de cada buffer: 240 x 320 x 2 bytes (RGB565) = 153.6 KB ≈ 150 KB
 * Tenemos 8 MB de PSRAM, así que es más que suficiente.
 */
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;
static TimerHandle_t tick_timer = NULL;

// =============================================================================
// CALLBACKS DE LVGL
// =============================================================================

/**
 * @brief Callback llamado por LVGL cuando tiene píxeles para enviar a pantalla.
 *
 * Flujo:
 * 1. LVGL finalizó de dibujar una región
 * 2. Llama a lvgl_port_flush_callback
 * 3. Nosotros extraemos el área (x1, y1, x2, y2) y el buffer de píxeles
 * 4. Usamos display_driver_draw_bitmap para enviar esos píxeles vía SPI
 * 5. Decimos a LVGL "listo, puedes continuar dibujando" (lv_disp_flush_ready)
 */
void lvgl_port_flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    // LVGL usa límites inclusivos; esp_lcd_panel_draw_bitmap usa el final
    // exclusivo. No enviar la conversión corrompe la última fila del buffer.
    int32_t x2 = area->x2 + 1;
    int32_t y2 = area->y2 + 1;

    // Extraemos los píxeles en formato RGB565 y los enviamos a la pantalla.
    // color_map ya está en el formato correcto que el ST7789T3 espera.
    esp_err_t ret = display_driver_draw_bitmap(x1, y1, x2, y2, (const void *)color_map);

    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Failed to draw bitmap: 0x%x", ret);
    }

    // Le decimos a LVGL: "Los píxeles ya fueron enviados, puedes continuar"
    lv_disp_flush_ready(drv);
}

/**
 * @brief Callback de timer para actualizar el reloj interno de LVGL.
 *
 * LVGL mantiene un contador de tiempo interno (en milisegundos) que utiliza para:
 * - Animaciones (transiciones suaves entre pantallas)
 * - Detectar pulsaciones largas en botones
 * - Temporizadores internos
 * - Control de refresco
 *
 * FreeRTOS llama a esta función cada 5 ms.
 */
void lvgl_port_tick_callback(TimerHandle_t timer)
{
    (void)timer;
    // Decimos a LVGL: "pasaron 5 ms más"
    lv_tick_inc(5);
}

// =============================================================================
// FUNCIÓN DE INICIALIZACIÓN
// =============================================================================

esp_err_t lvgl_port_init(void)
{
    NERA_LOGI(TAG, "Initializing LVGL port...");

    // =========================================================================
    // 1. ASIGNAR BUFFERS EN PSRAM
    // =========================================================================

    /**
     * ps_malloc: Malloc que fuerza la asignación en PSRAM (no en RAM interna).
     *
     * ¿Por qué PSRAM?
     * Dos buffers de 150 KB = 300 KB.
     * La RAM interna del ESP32-S3 es de ~520 KB (tras usar parte para FreeRTOS).
     * 300 KB sería casi todo lo disponible. Usamos PSRAM para dejar RAM libre
     * para el código de FreeRTOS, sensores, BLE, etc.
     */
    // Usar buffers DMA en RAM interna evita que el controlador SPI tenga que
    // copiar cada flush desde PSRAM. 20 líneas también coincide con el tamaño
    // de transferencia DMA declarado por la configuración del proyecto.
    const size_t buffer_pixels = NERA_LCD_WIDTH * 20;
    const size_t buffer_size = buffer_pixels * sizeof(lv_color_t);

    buf1 = (lv_color_t *)heap_caps_malloc(
        buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (buf1 == NULL) {
        NERA_LOGW(TAG, "DMA buffer 1 unavailable; trying PSRAM");
        buf1 = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
    }
    if (buf1 == NULL) {
        NERA_LOGE(TAG, "Failed to allocate buf1 (size: %d bytes)", buffer_size);
        return ESP_ERR_NO_MEM;
    }

    buf2 = (lv_color_t *)heap_caps_malloc(
        buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (buf2 == NULL) {
        NERA_LOGW(TAG, "DMA buffer 2 unavailable; trying PSRAM");
        buf2 = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
    }
    if (buf2 == NULL) {
        NERA_LOGE(TAG, "Failed to allocate buf2 (size: %d bytes)", buffer_size);
        heap_caps_free(buf1);
        return ESP_ERR_NO_MEM;
    }

    NERA_LOGI(TAG, "Allocated 2 LVGL draw buffers: %.1f KB each", buffer_size / 1024.0f);

    // =========================================================================
    // 2. INICIALIZAR LVGL
    // =========================================================================

    lv_init();

    // =========================================================================
    // 3. CONFIGURAR DRIVER DE PANTALLA
    // =========================================================================

    // Estructura que describe el buffer de dibujo
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, buffer_pixels);

    // Estructura que describe el driver de pantalla
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    // Configurar el driver para que use nuestros buffers y callback de flush
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = lvgl_port_flush_callback;
    disp_drv.hor_res = NERA_LCD_WIDTH;   // Resolución horizontal: 240 px
    disp_drv.ver_res = NERA_LCD_HEIGHT;  // Resolución vertical: 320 px

    // Registrar el driver para que LVGL lo use
    lv_disp_drv_register(&disp_drv);

    // =========================================================================
    // 4. CREAR TIMER PARA EL TICK DE LVGL
    // =========================================================================

    /**
     * FreeRTOS Timer: Un mecanismo que ejecuta una función cada cierto tiempo.
     *
     * ¿Por qué necesitamos un timer?
     * LVGL mantiene un reloj interno en milisegundos. Cada 5 ms, FreeRTOS
     * llama a lvgl_port_tick_callback, que incrementa ese reloj.
     * Sin esto, LVGL no sabría cuánto tiempo ha pasado y las animaciones
     * se congdelaría.
     *
     * Parámetros:
     * - "lvgl_tick": Nombre del timer (solo para debugging)
     * - 5: Ejecutar cada 5 milisegundos
     * - pdTRUE: Repetir indefinidamente (no one-shot)
     * - NULL: Argumento para la función (no necesitamos)
     * - lvgl_port_tick_callback: Función a ejecutar
     */
    tick_timer = xTimerCreate(
        "lvgl_tick",
        pdMS_TO_TICKS(5),
        pdTRUE,
        NULL,
        lvgl_port_tick_callback
    );

    if (tick_timer == NULL) {
        NERA_LOGE(TAG, "Failed to create LVGL tick timer");
        heap_caps_free(buf1);
        heap_caps_free(buf2);
        return ESP_ERR_NO_MEM;
    }

    // Iniciar el timer
    if (xTimerStart(tick_timer, pdMS_TO_TICKS(100)) != pdPASS) {
        NERA_LOGE(TAG, "Failed to start LVGL tick timer");
        heap_caps_free(buf1);
        heap_caps_free(buf2);
        xTimerDelete(tick_timer, pdMS_TO_TICKS(100));
        return ESP_FAIL;
    }

    NERA_LOGI(TAG, "LVGL port initialized successfully");
    NERA_LOGI(TAG, "Display: %dx%d px, Double buffering enabled", NERA_LCD_WIDTH, NERA_LCD_HEIGHT);

    return ESP_OK;
}
