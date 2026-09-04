/**
 * @file logger.h
 * @brief Sistema de logging organizado para NERA.
 *
 * ¿Qué es logging?
 * El "logging" es la práctica de imprimir mensajes informativos durante la
 * ejecución del firmware. En lugar de usar printf() genérico, ESP-IDF tiene
 * un sistema de logging con niveles (ERROR, WARN, INFO, DEBUG, VERBOSE) y
 * tags (etiquetas que identifican qué módulo generó el mensaje).
 *
 * Ejemplo de salida en el monitor serial:
 *   I (1234) NERA/DISPLAY: Pantalla inicializada correctamente. 240x320px
 *   W (2345) NERA/SENSOR: Sensor de frecuencia cardíaca no disponible
 *   E (3456) NERA/STORAGE: Error al escribir en Flash: ESP_ERR_NO_MEM
 *
 * Cada módulo de NERA define su propio tag usando las macros de este archivo.
 * Esto permite filtrar logs por módulo cuando hay mucho output.
 *
 * Niveles de log (de mayor a menor severidad):
 *   LOGE — Error: algo falló y puede afectar el funcionamiento
 *   LOGW — Warning: algo inusual pero no crítico
 *   LOGI — Info: operaciones normales importantes
 *   LOGD — Debug: información detallada para debugging
 *   LOGV — Verbose: máximo detalle, solo para debugging intenso
 */

#pragma once

#include "esp_log.h"
#include "nera_config.h"

// =============================================================================
// TAGS DE MÓDULOS
// =============================================================================
//
// Cada módulo tiene su propio tag que aparece en los mensajes de log.
// El formato es "NERA/NOMBRE_MODULO" para distinguirlos claramente.

#define NERA_TAG_MAIN       "NERA/MAIN"
#define NERA_TAG_DISPLAY    "NERA/DISPLAY"
#define NERA_TAG_TOUCH      "NERA/TOUCH"
#define NERA_TAG_UI         "NERA/UI"
#define NERA_TAG_SENSOR     "NERA/SENSOR"
#define NERA_TAG_HEALTH     "NERA/HEALTH"
#define NERA_TAG_SLEEP      "NERA/SLEEP"
#define NERA_TAG_STORAGE    "NERA/STORAGE"
#define NERA_TAG_BLE        "NERA/BLE"
#define NERA_TAG_POWER      "NERA/POWER"
#define NERA_TAG_IMU        "NERA/IMU"
#define NERA_TAG_BATTERY    "NERA/BATTERY"
#define NERA_TAG_STATE      "NERA/STATE"
#define NERA_TAG_LVGL       "NERA/LVGL"

// =============================================================================
// MACROS DE LOGGING
// =============================================================================
//
// Estas macros simplifican el uso del sistema de logging de ESP-IDF.
// En lugar de escribir ESP_LOGI("NERA/DISPLAY", "..."), escribís:
//   NERA_LOGI(NERA_TAG_DISPLAY, "...");
//
// La ventaja: si en el futuro cambiamos el sistema de logging, solo
// modificamos este archivo, no todos los archivos del proyecto.

/// Macro para mensajes de información (operaciones normales importantes).
#define NERA_LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)

/// Macro para advertencias (algo inusual pero no crítico).
#define NERA_LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)

/// Macro para errores (algo falló).
#define NERA_LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)

/// Macro para debug (información detallada para desarrollo).
#define NERA_LOGD(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)

/// Macro para verbose (máximo detalle).
#define NERA_LOGV(tag, fmt, ...) ESP_LOGV(tag, fmt, ##__VA_ARGS__)

// =============================================================================
// VERIFICACIÓN DE ERRORES ESP-IDF
// =============================================================================
//
// En ESP-IDF, muchas funciones devuelven un valor de tipo esp_err_t.
// esp_err_t es básicamente un número entero donde:
//   - ESP_OK (0)     = todo salió bien
//   - Cualquier otro = código de error
//
// La macro NERA_CHECK_ERROR verifica si hubo error y lo loguea automáticamente.

/// Verifica un resultado ESP-IDF. Si hay error, loguea y ejecuta `action`.
/// Ejemplo: NERA_CHECK_ERROR(spi_bus_initialize(...), goto error);
#define NERA_CHECK_ERROR(expr, action)                                          \
    do {                                                                         \
        esp_err_t _err = (expr);                                                 \
        if (_err != ESP_OK) {                                                    \
            NERA_LOGE(NERA_TAG_MAIN, "Error en %s:%d — %s (%s)",               \
                      __FILE__, __LINE__, #expr, esp_err_to_name(_err));        \
            action;                                                              \
        }                                                                        \
    } while(0)

/// Verifica un resultado ESP-IDF. Si hay error, loguea y retorna el error.
/// Útil dentro de funciones que devuelven esp_err_t.
#define NERA_RETURN_ON_ERROR(expr)                                              \
    do {                                                                         \
        esp_err_t _err = (expr);                                                 \
        if (_err != ESP_OK) {                                                    \
            NERA_LOGE(NERA_TAG_MAIN, "Error en %s:%d — %s (%s)",               \
                      __FILE__, __LINE__, #expr, esp_err_to_name(_err));        \
            return _err;                                                         \
        }                                                                        \
    } while(0)
