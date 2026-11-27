# Firmware activo: pantalla, tactil y lecturas reales de MAX30102/MPU6050/MLX90614.
if(CONFIG_NERA_ESPNOW_DIAGNOSTIC)
    list(APPEND NERA_RADIO_SOURCES
        "connectivity/espnow_diagnostic.cpp")
endif()
idf_component_register(
    SRCS
        ${NERA_RADIO_SOURCES}
        "connectivity/ble_manager.cpp"
        "connectivity/alert_delivery.cpp"
        "connectivity/user_alert_preferences.cpp"
        "services/time_service.cpp"
        "services/history_service.cpp"
        "app_main.cpp"
        "display/display_driver.cpp"
        "display/display_manager.cpp"
        "touch/touch_driver.cpp"
        "lvgl/lvgl_port.cpp"
        "lvgl/lvgl_task.cpp"
        "ui/ui_manager.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_common esp_system freertos esp_timer log esp_hw_support esp_lcd
             driver esp_driver_spi esp_driver_ledc esp_driver_gpio esp_driver_i2c lvgl sensores
             # ESP-IDF calcula dependencias antes de cargar todos los CONFIG.
             # Declararlas siempre; el inicio de radio sigue siendo condicional.
             esp_wifi esp_event esp_netif nvs_flash bt spiffs
)
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-error=deprecated-enum-enum-conversion")
# IMPORTANTE: conservar pool LVGL en PSRAM y buffers DMA del puerto existente.
idf_component_get_property(NERA_LVGL_TARGET lvgl COMPONENT_LIB)
target_include_directories(${NERA_LVGL_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${NERA_LVGL_TARGET} PRIVATE
    LV_MEM_POOL_INCLUDE="lvgl/nera_lvgl_memory.h"
    LV_MEM_POOL_ALLOC=nera_lvgl_pool_alloc
)
