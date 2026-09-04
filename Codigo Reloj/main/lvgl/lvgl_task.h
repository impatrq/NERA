/**
 * @file lvgl_task.h
 * @brief Tarea FreeRTOS para ejecutar el motor gráfico LVGL.
 *
 * Esta tarea es el corazón de la interfaz gráfica del smartwatch.
 * Ejecuta el bucle de eventos de LVGL continuamente:
 *   1. Procesa eventos (toques, timers)
 *   2. Anima widgets
 *   3. Redibuja lo que cambió
 *   4. Duerme hasta el próximo ciclo
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Función ejecutada por la tarea FreeRTOS de LVGL.
 *
 * Esta función se ejecuta en bucle infinito en su propio hilo de FreeRTOS.
 * Debe lanzarse con xTaskCreate.
 *
 * @param pvParameters Argumento que pasa FreeRTOS (no usado, NULL).
 */
void lvgl_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
