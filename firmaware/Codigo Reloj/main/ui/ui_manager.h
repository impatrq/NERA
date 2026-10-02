/**
 * @file ui_manager.h
 * @brief Gestor principal de pantallas LVGL para NERA.
 *
 * UIManager separa la construccion de pantallas de la tarea LVGL.
 * La tarea LVGL se encarga de ejecutar el motor grafico; este modulo se
 * encarga de decidir que pantalla existe, que datos muestra y como navega.
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa la interfaz grafica principal de NERA.
 *
 * Debe llamarse despues de lvgl_port_init(), porque necesita que LVGL ya
 * tenga un display registrado.
 *
 * @return ESP_OK si la UI pudo crearse correctamente.
 */
esp_err_t ui_manager_init(void);

#ifdef __cplusplus
}
#endif
