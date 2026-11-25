#include "services/history_service.h"

#include <atomic>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_random.h"
#include "esp_spiffs.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sensor_service.h"
#include "services/time_service.h"

namespace {
constexpr uint32_t CAPACITY = 7 * 24 * 60; // Una muestra por minuto, siete dias.
constexpr char PATH[] = "/history/samples_v1.bin";
constexpr char GENERATION_PATH[] = "/history/generation_v1.bin";
SemaphoreHandle_t mutex = nullptr;
FILE *file = nullptr;
uint32_t latest_sequence = 0;
uint32_t first_sequence_this_boot = 1;
uint32_t generation = 0;
std::atomic<bool> ready{false};

uint32_t checksum(const HistoryPoint &point) {
    const auto *bytes = reinterpret_cast<const uint8_t *>(&point);
    uint32_t hash = 2166136261U;
    for (size_t i = 0; i < offsetof(HistoryPoint, checksum); ++i)
        hash = (hash ^ bytes[i]) * 16777619U;
    return hash;
}
bool valid(const HistoryPoint &point) {
    return point.sequence != 0 && point.checksum == checksum(point) &&
           (point.bpm == 0 || (point.bpm >= 30 && point.bpm <= 240)) &&
           (point.object_deci_c == INT16_MIN ||
            (point.object_deci_c >= -700 && point.object_deci_c <= 3800));
}
long position(uint32_t sequence) {
    return static_cast<long>(((sequence - 1) % CAPACITY) * sizeof(HistoryPoint));
}
bool read_sequence(uint32_t sequence, HistoryPoint *out) {
    if (fseek(file, position(sequence), SEEK_SET) != 0) return false;
    HistoryPoint point = {};
    if (fread(&point, sizeof(point), 1, file) != 1 || !valid(point) ||
        point.sequence != sequence) return false;
    *out = point;
    return true;
}
bool write_point(HistoryPoint point) {
    point.checksum = checksum(point);
    if (fseek(file, position(point.sequence), SEEK_SET) != 0 ||
        fwrite(&point, sizeof(point), 1, file) != 1 || fflush(file) != 0)
        return false;
    return true;
}
}

esp_err_t history_service_init(void) {
    if (ready.load()) return ESP_OK;
    mutex = xSemaphoreCreateMutex();
    if (!mutex) return ESP_ERR_NO_MEM;
    const esp_vfs_spiffs_conf_t conf = {
        .base_path = "/history", .partition_label = "littlefs",
        .max_files = 3, .format_if_mount_failed = true,
    };
    const esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
        ESP_LOGE("NERA/HISTORY", "No se pudo montar historial: %s", esp_err_to_name(err));
        return err;
    }
    file = fopen(PATH, "r+b");
    if (!file) file = fopen(PATH, "w+b");
    if (!file) {
        esp_vfs_spiffs_unregister("littlefs");
        vSemaphoreDelete(mutex);
        mutex = nullptr;
        return ESP_FAIL;
    }
    FILE *generation_file = fopen(GENERATION_PATH, "rb");
    if (generation_file) {
        if (fread(&generation, sizeof(generation), 1, generation_file) != 1)
            generation = 0;
        fclose(generation_file);
    }
    if (generation == 0) {
        generation = esp_random();
        if (generation == 0) generation = 1;
        generation_file = fopen(GENERATION_PATH, "wb");
        if (!generation_file || fwrite(&generation, sizeof(generation), 1, generation_file) != 1 ||
            fflush(generation_file) != 0) {
            if (generation_file) fclose(generation_file);
            fclose(file); file = nullptr;
            esp_vfs_spiffs_unregister("littlefs");
            vSemaphoreDelete(mutex); mutex = nullptr;
            return ESP_FAIL;
        }
        fclose(generation_file);
    }
    HistoryPoint point = {};
    for (uint32_t slot = 0; slot < CAPACITY; ++slot) {
        if (fseek(file, static_cast<long>(slot * sizeof(point)), SEEK_SET) != 0 ||
            fread(&point, sizeof(point), 1, file) != 1) break;
        if (valid(point) && point.sequence > latest_sequence)
            latest_sequence = point.sequence;
    }
    clearerr(file);
    first_sequence_this_boot = latest_sequence + 1;
    ready.store(true);
    ESP_LOGI("NERA/HISTORY", "Historial listo; ultimo=%lu, capacidad=%lu minutos",
             static_cast<unsigned long>(latest_sequence), static_cast<unsigned long>(CAPACITY));
    return ESP_OK;
}

bool history_service_is_ready(void) { return ready.load(); }
uint32_t history_service_latest_sequence(void) {
    if (!ready.load() || xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) return 0;
    const uint32_t sequence = latest_sequence;
    xSemaphoreGive(mutex);
    return sequence;
}
uint32_t history_service_generation(void) { return ready.load() ? generation : 0; }

void history_service_task(void *) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(60000));
        if (!ready.load()) continue;
        SensorSnapshot sample = {};
        if (sensor_service_get_snapshot(&sample) != ESP_OK) continue;
        HistoryPoint point = {};
        point.bpm = sample.heart_valid && isfinite(sample.bpm) &&
                    sample.bpm >= 30 && sample.bpm <= 240
                    ? static_cast<int16_t>(lroundf(sample.bpm)) : 0;
        point.object_deci_c = sample.mlx_valid && isfinite(sample.object_celsius) &&
                              sample.object_celsius >= -70 && sample.object_celsius <= 380
                              ? static_cast<int16_t>(lroundf(sample.object_celsius * 10))
                              : INT16_MIN;
        if (!point.bpm && point.object_deci_c == INT16_MIN) continue;
        point.epoch_seconds = time_service_epoch();
        point.uptime_seconds = static_cast<uint32_t>(esp_timer_get_time() / 1000000);
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) != pdTRUE) continue;
        point.sequence = latest_sequence + 1;
        if (write_point(point)) latest_sequence = point.sequence;
        else ESP_LOGW("NERA/HISTORY", "No se pudo guardar muestra %lu",
                      static_cast<unsigned long>(point.sequence));
        xSemaphoreGive(mutex);
    }
}

void history_service_backfill_current_boot(void) {
    if (!ready.load() || !time_service_is_synced() ||
        xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) != pdTRUE) return;
    uint32_t updated = 0;
    const uint32_t oldest = latest_sequence >= CAPACITY
        ? latest_sequence - CAPACITY + 1 : 1;
    const uint32_t first = first_sequence_this_boot > oldest
        ? first_sequence_this_boot : oldest;
    for (uint32_t seq = first; seq <= latest_sequence; ++seq) {
        HistoryPoint point = {};
        if (!read_sequence(seq, &point) || point.epoch_seconds != 0) continue;
        point.epoch_seconds = time_service_epoch_for_uptime(
            static_cast<int64_t>(point.uptime_seconds) * 1000000);
        if (point.epoch_seconds != 0 && write_point(point)) ++updated;
    }
    xSemaphoreGive(mutex);
    if (updated) ESP_LOGI("NERA/HISTORY", "Fechadas %lu muestras de este arranque",
                          static_cast<unsigned long>(updated));
}

bool history_service_get_after(uint32_t sequence, HistoryPoint *out) {
    if (!out || !ready.load() ||
        xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) return false;
    const uint32_t oldest = latest_sequence >= CAPACITY
        ? latest_sequence - CAPACITY + 1 : 1;
    uint32_t first = sequence < oldest ? oldest : sequence + 1;
    bool found = false;
    for (uint32_t seq = first; seq <= latest_sequence; ++seq) {
        if (read_sequence(seq, out)) { found = true; break; }
    }
    xSemaphoreGive(mutex);
    return found;
}
