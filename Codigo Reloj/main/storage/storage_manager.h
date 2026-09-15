/** Persistencia NVS: carga al arrancar y guardado en una tarea independiente.
 * La UI publica preferencias en una cola; no realiza commits desde callbacks. */
#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t brightness;
    bool notifications_enabled;
    bool vibration_enabled;
    bool power_saver_enabled;
} NeraUserSettings;

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t storage_manager_init(void);
void storage_manager_task(void *arg);
esp_err_t storage_manager_load_settings(NeraUserSettings *settings);
esp_err_t storage_manager_save_settings(const NeraUserSettings *settings);
// Publicacion no bloqueante: la tarea storage hace el commit NVS.
esp_err_t storage_manager_request_settings(const NeraUserSettings *settings);
esp_err_t storage_manager_settings_status(void);

#ifdef __cplusplus
}
#endif
