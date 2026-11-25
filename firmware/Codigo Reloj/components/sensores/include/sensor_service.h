/** Lecturas publicadas por una tarea aparte; LVGL nunca accede al bus I2C. */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

struct SensorSnapshot {
    float bpm;
    float spo2; // Estimación experimental, no calibrada para uso clínico.
    float object_celsius;
    float ambient_celsius;
    float acceleration_g;  // Modulo del vector del MPU6050, incluye gravedad.
    float rotation_dps;    // Modulo de velocidad angular.
    bool mlx_valid;
    bool mpu_valid;
    bool heart_valid;
    bool spo2_valid;
    bool heart_held; // BPM reciente conservado brevemente, no una nueva medicion.
    bool max_online;
    bool max_contact;  // Nivel IR suficiente; no demuestra por si solo un pulso.
    bool max_saturated;
    uint32_t max_ir;
    int64_t heart_updated_us;
    int64_t spo2_updated_us;
    int64_t mlx_updated_us;
    int64_t mpu_updated_us;
};

esp_err_t sensor_service_start(void);
esp_err_t sensor_service_get_snapshot(SensorSnapshot *snapshot);
