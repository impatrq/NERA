idf_component_register(SRCS "main.c"
                         "display.c"
                         "pantalla_principal.c"
                         "psram_mgr.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_psram
                             esp_driver_gpio
                             esp_driver_i2c
                             esp_driver_spi
                             esp_lcd
                             esp_lvgl_port
                             esp_lcd_touch_cst816s)
