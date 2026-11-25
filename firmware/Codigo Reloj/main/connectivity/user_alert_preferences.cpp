#include "connectivity/user_alert_preferences.h"
#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"
#include "esp_log.h"

namespace nera_preferences {
namespace {
std::atomic<uint32_t> current{0x00000101};
std::atomic<bool> pending{false};
QueueHandle_t requests=nullptr;
uint32_t pack(Config c) {
    return c.version | (uint32_t(c.enabled)<<8) | (uint32_t(c.sensitivity)<<16) |
           (uint32_t(c.exercise)<<24);
}
Config unpack(uint32_t p) { return {uint8_t(p),uint8_t(p>>8),uint8_t(p>>16),uint8_t(p>>24)}; }
void save_task(void *) {
    Config c;
    while (xQueueReceive(requests,&c,portMAX_DELAY)==pdTRUE) {
        const uint8_t bytes[]={c.version,c.enabled,c.sensitivity,c.exercise};
        nvs_handle_t handle;
        esp_err_t err=nvs_open("nera_alert",NVS_READWRITE,&handle);
        if (err==ESP_OK) {
            err=nvs_set_blob(handle,"policy",bytes,sizeof(bytes));
            if (err==ESP_OK) err=nvs_commit(handle);
            nvs_close(handle);
        }
        if (err==ESP_OK) {
            current.store(pack(c));
            ESP_LOGI("NERA/PREF","Guardado: avisos=%u sensibilidad=%u ejercicio=%u",
                     c.enabled,c.sensitivity,c.exercise);
        } else ESP_LOGE("NERA/PREF","No se aplicaron ajustes: %s",esp_err_to_name(err));
        pending.store(false);
    }
}
}
Config get() { return unpack(current.load()); }
bool saving() { return pending.load(); }
esp_err_t request(Config c) {
    if (!valid(c)) return ESP_ERR_INVALID_ARG;
    if (!requests) return ESP_ERR_INVALID_STATE;
    if (pending.exchange(true)) return ESP_ERR_INVALID_STATE;
    if (pack(c)==current.load()) { pending.store(false); return ESP_OK; }
    if (xQueueOverwrite(requests,&c)==pdTRUE) return ESP_OK;
    pending.store(false); return ESP_FAIL;
}
esp_err_t init() {
    if (requests) return ESP_OK;
    nvs_handle_t handle;
    esp_err_t err=nvs_open("nera_alert",NVS_READONLY,&handle);
    if (err==ESP_OK) {
        uint8_t b[4]={}; size_t size=sizeof(b);
        err=nvs_get_blob(handle,"policy",b,&size);
        nvs_close(handle);
        Config c={b[0],b[1],b[2],b[3]};
        if (err==ESP_OK && size==sizeof(b) && valid(c)) current.store(pack(c));
        else if (err!=ESP_ERR_NVS_NOT_FOUND)
            ESP_LOGW("NERA/PREF","Preferencias ausentes/incompatibles; valores de laboratorio");
    } else if (err!=ESP_ERR_NVS_NOT_FOUND) return err;
    requests=xQueueCreate(1,sizeof(Config));
    if (!requests) return ESP_ERR_NO_MEM;
    // IMPORTANTE: NVS fuera del callback BLE y de LVGL.
    if (xTaskCreate(save_task,"nera_preferences",3072,nullptr,2,nullptr)!=pdPASS) {
        vQueueDelete(requests); requests=nullptr; return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}
}
