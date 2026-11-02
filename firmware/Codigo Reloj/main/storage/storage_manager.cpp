/** NVS compatible con 3A: ultimas 60 muestras, no archivo continuo.
 * Storage escribe fuera de LVGL cada minuto si cambio la serie, incluso llena. */
#include "storage/storage_manager.h"
#include "core/app_state.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"
#include "utils/logger.h"
#include <atomic>
#include <math.h>
#include <string.h>
namespace {
// El firmware anterior solo tenia sensores mock. Sus claves se conservan como demo.
const char *history_space = NERA_USE_MOCK_SENSORS ? "nera" : "nera_real";
struct HistoryBlob { uint8_t count; float values[NERA_HISTORY_BUFFER_SIZE]; };
QueueHandle_t settings_queue = nullptr;
std::atomic<esp_err_t> status{ESP_OK};
bool valid(const HistoryBlob &b, bool heart) {
    if (b.count > NERA_HISTORY_BUFFER_SIZE) return false;
    for (unsigned i = 0; i < b.count; ++i)
        if (!isfinite(b.values[i]) || (heart && b.values[i] <= 0)) return false;
    return true;
}
esp_err_t write_blob(const char *key, const void *data, size_t size, const char *space = "nera") {
    nvs_handle_t h;
    esp_err_t err = nvs_open(space, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_blob(h, key, data, size);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}
}
esp_err_t storage_manager_load_settings(NeraUserSettings *s) {
    if (!s) return ESP_ERR_INVALID_ARG;
    *s = {NERA_BL_DEFAULT, true, true, false};
    nvs_handle_t h;
    esp_err_t err = nvs_open("nera", NVS_READONLY, &h);
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (err != ESP_OK) return err;
    // Bytes explicitos para no interpretar un bool corrupto desde Flash.
    uint8_t bytes[4] = {};
    size_t size = sizeof(bytes);
    err = nvs_get_blob(h, "settings", bytes, &size);
    nvs_close(h);
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (err != ESP_OK) return err;
    if (size != sizeof(bytes) || bytes[1] > 1 || bytes[2] > 1 || bytes[3] > 1) return ESP_ERR_INVALID_SIZE;
    *s = {bytes[0], bytes[1] != 0, bytes[2] != 0, bytes[3] != 0};
    return ESP_OK;
}
esp_err_t storage_manager_save_settings(const NeraUserSettings *s) {
    if (!s) return ESP_ERR_INVALID_ARG;
    const uint8_t bytes[] = {s->brightness, (uint8_t)s->notifications_enabled,
                            (uint8_t)s->vibration_enabled, (uint8_t)s->power_saver_enabled};
    return write_blob("settings", bytes, sizeof(bytes));
}
esp_err_t storage_manager_request_settings(const NeraUserSettings *s) {
    if (!s || !settings_queue) return ESP_ERR_INVALID_STATE;
    status.store(ESP_ERR_NOT_FINISHED);
    return xQueueOverwrite(settings_queue, s) == pdTRUE ? ESP_OK : ESP_FAIL;
}
esp_err_t storage_manager_settings_status(void) { return status.load(); }
esp_err_t storage_manager_init(void) {
    if (!settings_queue) settings_queue = xQueueCreate(1, sizeof(NeraUserSettings));
    if (!settings_queue) return ESP_ERR_NO_MEM;
    nvs_handle_t h;
    esp_err_t err = nvs_open(history_space, NVS_READONLY, &h);
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (err != ESP_OK) return err;
    const char *keys[] = {"heart_hist", "temp_hist"};
    for (unsigned i = 0; i < 2; ++i) {
        HistoryBlob b = {};
        size_t size = sizeof(b);
        err = nvs_get_blob(h, keys[i], &b, &size);
        if (err == ESP_ERR_NVS_NOT_FOUND) continue;
        if (err != ESP_OK || size != sizeof(b) || !valid(b, i == 0)) {
            NERA_LOGW(NERA_TAG_STORAGE, "%s invalido; ignorado", keys[i]);
            continue;
        }
        err = i == 0 ? app_state_restore_heart_history(b.values, b.count) :
                       app_state_restore_temp_history(b.values, b.count);
        if (err == ESP_OK) NERA_LOGI(NERA_TAG_STORAGE, "%s restaurado: %u", keys[i], b.count);
    }
    nvs_close(h);
    return ESP_OK;
}
void storage_manager_task(void *arg) {
    (void)arg;
    HistoryBlob saved[2] = {};
    TickType_t last_save = xTaskGetTickCount();
    while (true) {
        NeraUserSettings s;
        if (settings_queue && xQueueReceive(settings_queue, &s, 0) == pdTRUE) {
            esp_err_t err = storage_manager_save_settings(&s);
            if (uxQueueMessagesWaiting(settings_queue) == 0) status.store(err);
            if (err != ESP_OK) NERA_LOGW(NERA_TAG_STORAGE, "Ajustes: %s", esp_err_to_name(err));
        }
        const TickType_t now = xTaskGetTickCount();
        if (now - last_save >= pdMS_TO_TICKS(60000)) {
            last_save = now;
            NeraAppState state = {};
            if (app_state_get(&state) == ESP_OK) {
                for (unsigned i = 0; i < 2; ++i) {
                    HistoryBlob b = {};
                    b.count = i == 0 ? state.heart_history_count : state.temp_history_count;
                    memcpy(b.values, i == 0 ? state.heart_history : state.temp_history, sizeof(b.values));
                    if (memcmp(&saved[i], &b, sizeof(b)) == 0 || !valid(b, i == 0)) continue;
                    esp_err_t err = write_blob(i == 0 ? "heart_hist" : "temp_hist", &b, sizeof(b), history_space);
                    if (err == ESP_OK) saved[i] = b;
                    else NERA_LOGW(NERA_TAG_STORAGE, "Historial: %s", esp_err_to_name(err));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
