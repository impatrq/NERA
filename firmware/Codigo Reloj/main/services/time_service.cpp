#include "services/time_service.h"

#include <atomic>
#include <stdio.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_timer.h"

namespace {
constexpr uint32_t MIN_EPOCH = 1704067200; // 2024-01-01 UTC.
constexpr uint32_t MAX_EPOCH = 4102444800U; // 2100-01-01 UTC.
std::atomic<bool> synced{false};
std::atomic<int16_t> offset_minutes{0};
std::atomic<int64_t> epoch_at_boot_us{0};
}

esp_err_t time_service_sync(uint32_t epoch_seconds, int16_t utc_offset_minutes) {
    if (epoch_seconds < MIN_EPOCH || epoch_seconds >= MAX_EPOCH ||
        utc_offset_minutes < -720 || utc_offset_minutes > 840)
        return ESP_ERR_INVALID_ARG;
    timeval now = {static_cast<time_t>(epoch_seconds), 0};
    if (settimeofday(&now, nullptr) != 0) return ESP_FAIL;
    const int64_t uptime_us = esp_timer_get_time();
    epoch_at_boot_us.store(static_cast<int64_t>(epoch_seconds) * 1000000 - uptime_us);
    offset_minutes.store(utc_offset_minutes);
    synced.store(true);
    ESP_LOGI("NERA/TIME", "Hora sincronizada desde telefono; UTC=%lu, offset=%d min",
             static_cast<unsigned long>(epoch_seconds), utc_offset_minutes);
    return ESP_OK;
}

bool time_service_is_synced(void) { return synced.load(); }
uint32_t time_service_epoch(void) {
    if (!synced.load()) return 0;
    timeval now = {};
    gettimeofday(&now, nullptr);
    return now.tv_sec >= MIN_EPOCH && now.tv_sec < MAX_EPOCH
        ? static_cast<uint32_t>(now.tv_sec) : 0;
}
int16_t time_service_utc_offset_minutes(void) { return offset_minutes.load(); }

bool time_service_local_hhmm(char out[6]) {
    if (!out) return false;
    const uint32_t epoch = time_service_epoch();
    if (!epoch) return false;
    int64_t local = static_cast<int64_t>(epoch) +
                    static_cast<int64_t>(offset_minutes.load()) * 60;
    int32_t seconds_of_day = static_cast<int32_t>(local % 86400);
    if (seconds_of_day < 0) seconds_of_day += 86400;
    snprintf(out, 6, "%02ld:%02ld", static_cast<long>(seconds_of_day / 3600),
             static_cast<long>((seconds_of_day / 60) % 60));
    return true;
}

uint32_t time_service_epoch_for_uptime(int64_t uptime_us) {
    if (!synced.load() || uptime_us < 0) return 0;
    const int64_t epoch_us = epoch_at_boot_us.load() + uptime_us;
    const int64_t epoch = epoch_us / 1000000;
    return epoch >= MIN_EPOCH && epoch < MAX_EPOCH ? static_cast<uint32_t>(epoch) : 0;
}
