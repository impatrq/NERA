#include "psram_mgr.h"

#include "esp_heap_caps.h"
#include "esp_log.h"

static const char *TAG = "nera_psram";

esp_err_t nera_psram_verificar(void)
{
    size_t libre = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "PSRAM libre: %u bytes", (unsigned)libre);
    return ESP_OK;
}
