#include "connectivity/ble_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/logger.h"

static bool s_ready = false;

esp_err_t ble_manager_init(void)
{
    // La capa queda preparada; el perfil GATT se agrega cuando exista el protocolo del telefono.
    s_ready = true;
    NERA_LOGI(NERA_TAG_BLE, "BLEManager listo; perfil GATT pendiente de integracion");
    return ESP_OK;
}

bool ble_manager_is_ready(void)
{
    return s_ready;
}

void ble_manager_task(void *arg)
{
    (void)arg;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
