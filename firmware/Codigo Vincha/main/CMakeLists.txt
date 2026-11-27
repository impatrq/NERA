idf_component_register(SRCS "main.c" "gpio5_signal_test.c"
                       REQUIRES esp_wifi esp_event esp_netif nvs_flash esp_driver_gpio esp_driver_ledc esp_timer)
