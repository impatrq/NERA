# =============================================================================
# NERA Smartwatch — main/CMakeLists.txt
# =============================================================================

idf_component_register(
    SRCS
        # Punto de entrada principal
        "app_main.cpp"

        # Módulo Core: estado global y comunicación
        "core/app_state.cpp"
        "core/event_bus.cpp"

        # Módulo Display: driver ST7789T3 y control de backlight PWM
        "display/display_driver.cpp"
        "display/display_manager.cpp"
        "touch/touch_driver.cpp"
        "sensors/heart_rate/heart_rate_filter.cpp"
        "sensors/heart_rate/mock_heart_rate_sensor.cpp"
        "sensors/temperature/mock_temperature_sensor.cpp"

        # Módulo LVGL: motor gráfico y tarea de UI
        "lvgl/lvgl_port.cpp"
        "lvgl/lvgl_task.cpp"
        "ui/ui_manager.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        esp_common
        esp_system
        freertos
        nvs_flash
        esp_timer
        log
        esp_hw_support
        esp_lcd
        driver
        esp_driver_spi
        esp_driver_ledc
        esp_driver_gpio
        esp_driver_i2c
        lvgl
)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-error=deprecated-enum-enum-conversion")
