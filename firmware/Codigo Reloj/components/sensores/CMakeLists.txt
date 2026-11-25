# FUNCIONAL: agrupa adquisicion, drivers y procesamiento. La UI no lee I2C.
idf_component_register(
    SRCS "sensor_service.cpp"
         "drivers/i2c_bus.c"
         "drivers/mpu6050.c"
         "drivers/mlx90614.c"
         "drivers/max30102.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "." "drivers"
    REQUIRES esp_common esp_timer freertos driver esp_driver_i2c esp_driver_gpio log
)
