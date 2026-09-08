#include "storage/storage_manager.h"

#include <string.h>

#include "core/app_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nera_config.h"
#include "nvs.h"
#include "utils/logger.h"

static constexpr char NERA_STORAGE_NAMESPACE[] = "nera";
static constexpr char NERA_HEART_HISTORY_KEY[] = "heart_hist";
static constexpr char NERA_TEMP_HISTORY_KEY[] = "temp_hist";
static constexpr char NERA_SETTINGS_KEY[] = "settings";
static constexpr uint32_t NERA_STORAGE_SAVE_INTERVAL_MS = 60000;
static constexpr uint8_t NERA_STORAGE_BATCH_SIZE = 10;

typedef struct {
    uint8_t count;
    float values[NERA_HISTORY_BUFFER_SIZE];
} HeartHistoryBlob;

typedef struct {
    uint8_t count;
    float values[NERA_HISTORY_BUFFER_SIZE];
} TempHistoryBlob;

static esp_err_t save_heart_history(const NeraAppState *state)
{
    if (state == nullptr || state->heart_history_count > NERA_HISTORY_BUFFER_SIZE) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(NERA_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    HeartHistoryBlob blob = {};
    blob.count = state->heart_history_count;
    memcpy(blob.values, state->heart_history, sizeof(blob.values));

    err = nvs_set_blob(handle, NERA_HEART_HISTORY_KEY, &blob, sizeof(blob));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

static esp_err_t save_temp_history(const NeraAppState *state)
{
    if (state == nullptr || state->temp_history_count > NERA_HISTORY_BUFFER_SIZE) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(NERA_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    TempHistoryBlob blob = {};
    blob.count = state->temp_history_count;
    memcpy(blob.values, state->temp_history, sizeof(blob.values));
    err = nvs_set_blob(handle, NERA_TEMP_HISTORY_KEY, &blob, sizeof(blob));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

esp_err_t storage_manager_load_settings(NeraUserSettings *settings)
{
    if (settings == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    *settings = {
        .brightness = NERA_BL_DEFAULT,
        .notifications_enabled = true,
        .vibration_enabled = true,
        .power_saver_enabled = false,
    };

    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(NERA_STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK;
    }
    if (err != ESP_OK) {
        return err;
    }

    size_t size = sizeof(*settings);
    err = nvs_get_blob(handle, NERA_SETTINGS_KEY, settings, &size);
    nvs_close(handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK;
    }
    if (err != ESP_OK || size != sizeof(*settings)) {
        *settings = {};
        settings->brightness = NERA_BL_DEFAULT;
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

esp_err_t storage_manager_save_settings(const NeraUserSettings *settings)
{
    if (settings == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(NERA_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        err = nvs_set_blob(handle, NERA_SETTINGS_KEY, settings, sizeof(*settings));
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    if (handle != 0) {
        nvs_close(handle);
    }
    return err;
}

esp_err_t storage_manager_init(void)
{
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(NERA_STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        NERA_LOGI(NERA_TAG_STORAGE, "Sin historial persistente; iniciando vacio");
        return ESP_OK;
    }
    if (err != ESP_OK) {
        NERA_LOGE(NERA_TAG_STORAGE, "No se pudo abrir NVS: %s", esp_err_to_name(err));
        return err;
    }

    HeartHistoryBlob blob = {};
    size_t blob_size = sizeof(blob);
    err = nvs_get_blob(handle, NERA_HEART_HISTORY_KEY, &blob, &blob_size);
    nvs_close(handle);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        NERA_LOGI(NERA_TAG_STORAGE, "Sin historial BPM persistente; iniciando vacio");
    } else if (err != ESP_OK || blob_size != sizeof(blob) ||
               blob.count > NERA_HISTORY_BUFFER_SIZE) {
        NERA_LOGW(NERA_TAG_STORAGE, "Historial persistente invalido; ignorando datos");
        return ESP_OK;
    } else {
        err = app_state_restore_heart_history(blob.values, blob.count);
        if (err == ESP_OK) {
            NERA_LOGI(NERA_TAG_STORAGE, "Historial BPM restaurado: %u muestras", blob.count);
        }
    }

    nvs_handle_t temp_handle = 0;
    if (nvs_open(NERA_STORAGE_NAMESPACE, NVS_READONLY, &temp_handle) == ESP_OK) {
        TempHistoryBlob temp_blob = {};
        size_t temp_size = sizeof(temp_blob);
        esp_err_t temp_err = nvs_get_blob(temp_handle, NERA_TEMP_HISTORY_KEY,
                                           &temp_blob, &temp_size);
        nvs_close(temp_handle);
        if (temp_err == ESP_OK && temp_size == sizeof(temp_blob) &&
            temp_blob.count <= NERA_HISTORY_BUFFER_SIZE) {
            app_state_restore_temp_history(temp_blob.values, temp_blob.count);
            NERA_LOGI(NERA_TAG_STORAGE, "Historial temperatura restaurado: %u muestras",
                      temp_blob.count);
        }
    }
    return err;
}

void storage_manager_task(void *arg)
{
    (void)arg;
    uint8_t last_saved_count = 0;
    uint8_t last_saved_temp_count = 0;
    TickType_t last_save_tick = xTaskGetTickCount();

    while (true) {
        NeraAppState state = {};
        if (app_state_get(&state) == ESP_OK) {
            const uint8_t count = state.heart_history_count;
            const uint8_t temp_count = state.temp_history_count;
            const TickType_t now = xTaskGetTickCount();
            const bool batch_ready = count >= last_saved_count + NERA_STORAGE_BATCH_SIZE;
            const bool interval_ready = (now - last_save_tick) >=
                                        pdMS_TO_TICKS(NERA_STORAGE_SAVE_INTERVAL_MS);

            if (count > last_saved_count && (batch_ready || interval_ready)) {
                esp_err_t err = save_heart_history(&state);
                if (err == ESP_OK) {
                    last_saved_count = count;
                    last_save_tick = now;
                    NERA_LOGI(NERA_TAG_STORAGE, "Historial BPM guardado: %u muestras", count);
                } else {
                    NERA_LOGW(NERA_TAG_STORAGE, "No se pudo guardar historial: %s",
                              esp_err_to_name(err));
                }
            }

            const bool temp_batch_ready = temp_count >= last_saved_temp_count + NERA_STORAGE_BATCH_SIZE;
            if (temp_count > last_saved_temp_count && (temp_batch_ready || interval_ready)) {
                esp_err_t err = save_temp_history(&state);
                if (err == ESP_OK) {
                    last_saved_temp_count = temp_count;
                    last_save_tick = now;
                    NERA_LOGI(NERA_TAG_STORAGE, "Historial temperatura guardado: %u muestras",
                              temp_count);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(NERA_SENSOR_UPDATE_INTERVAL_MS));
    }
}
