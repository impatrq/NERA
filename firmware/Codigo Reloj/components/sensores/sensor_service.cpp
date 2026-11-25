#include "sensor_service.h"

#include <math.h>
#include "driver/gpio.h"
#include "i2c_bus.h"
#include "mpu6050.h"
#include "mlx90614.h"
#include "max30102.h"
#include "processing/heart_rate_estimator.h"
#include "processing/spo2_estimator.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

namespace {
constexpr uint32_t SAMPLE_INTERVAL_MS = 20;
constexpr int64_t AUX_INTERVAL_US = 1000000;
constexpr int64_t MAX_SAMPLE_AGE_US = 3000000;
constexpr int64_t MAX_RETRY_US = 5000000, MAX_DIAGNOSTIC_US = 2000000;
constexpr int64_t SENSOR_STATUS_INTERVAL_US = 5000000;
SemaphoreHandle_t snapshot_mutex = nullptr;
SensorSnapshot current = {};
bool started = false;

void publish_heart(const HeartRateEstimator &estimator, const SpO2Estimator &oxygen,
                   bool sensor_ok, uint32_t ir,
                   bool signal_recent, int64_t latest_sample_us) {
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(20)) != pdTRUE) return;
    current.heart_valid = sensor_ok && estimator.valid();
    // El PBA puede rechazar intervalos de BPM aun con rojo e IR concordantes.
    // La calidad de la SpO2 se evalua en su propia ventana optica.
    current.spo2_valid = sensor_ok && oxygen.valid() && signal_recent;
    current.heart_held = current.heart_valid && estimator.holding();
    current.max_online = sensor_ok;
    current.max_ir = ir;
    current.max_contact = sensor_ok && signal_recent && ir>=HeartRateEstimator::CONTACT_IR_MIN;
    current.max_saturated = sensor_ok && signal_recent && ir>=HeartRateEstimator::SATURATION_IR;
    if (current.heart_valid) {
        current.bpm = estimator.bpm();
        // FUNCIONAL: conservar la fecha de la ultima medicion, no del repintado.
        current.heart_updated_us = latest_sample_us-
            (int64_t)estimator.samples_since_valid()*10000;
    }
    if (current.spo2_valid) {
        current.spo2 = oxygen.value();
        current.spo2_updated_us = latest_sample_us;
    }
    xSemaphoreGive(snapshot_mutex);
}

void publish_mlx(const mlx90614_data_t &sample, esp_err_t result) {
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(20)) != pdTRUE) return;
    current.mlx_valid = result == ESP_OK;
    if (current.mlx_valid) {
        current.object_celsius = sample.mlx_object_temp;
        current.ambient_celsius = sample.mlx_amb_temp;
        current.mlx_updated_us = esp_timer_get_time();
    }
    xSemaphoreGive(snapshot_mutex);
}

void publish_mpu(const mpu6050_data_t &sample, esp_err_t result) {
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(20)) != pdTRUE) return;
    current.mpu_valid = result == ESP_OK;
    if (current.mpu_valid) {
        // Rangos por defecto del MPU6050: 16384 LSB/g y 131 LSB/(grados/s).
        const float ax = sample.ax / 16384.0f, ay = sample.ay / 16384.0f, az = sample.az / 16384.0f;
        const float gx = sample.gx / 131.0f, gy = sample.gy / 131.0f, gz = sample.gz / 131.0f;
        current.acceleration_g = sqrtf(ax * ax + ay * ay + az * az);
        current.rotation_dps = sqrtf(gx * gx + gy * gy + gz * gz);
        current.mpu_updated_us = esp_timer_get_time();
    }
    xSemaphoreGive(snapshot_mutex);
}

void sensor_task(void *) {
    bool bus_ready = false, mpu_ready = false;
    bool previous_mlx_valid = false, previous_mpu_valid = false;
    bool mlx_status_logged = false, mpu_status_logged = false;
    bool max_ready = false, max_status_logged = false, previous_max_ready = false;
    int64_t last_max_attempt_us = -MAX_RETRY_US;
    int64_t last_max_sample_us = 0, last_aux_us = -AUX_INTERVAL_US;
    int64_t next_max_diagnostic_us = 0;
    int64_t next_sensor_status_us = 0;
    uint32_t last_ir=0, last_red=0, ir_min=UINT32_MAX, ir_max=0;
    unsigned window_samples=0, fifo_errors=0;
    unsigned max_samples_since_status = 0;
    HeartRateEstimator estimator;
    static SpO2Estimator oxygen;
    for (;;) {
        if (!bus_ready) {
            // Usar la inicializacion provista por el componente agregado.
            bus_ready = init_i2c() == ESP_OK;
            if (!bus_ready) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
        }
        const int64_t now = esp_timer_get_time();
        if (!max_ready && now - last_max_attempt_us >= MAX_RETRY_US) {
            last_max_attempt_us = now;
            const esp_err_t result=max30102_init(MLX_MPU6050_I2C_PORT);
            max_ready = result == ESP_OK;
            if (!max_ready)
                ESP_LOGW("NERA/MAX","INIT 0x57: %s; SDA(GPIO%d)=%d SCL(GPIO%d)=%d; reintento 5 s",
                    esp_err_to_name(result),MLX_MPU6050_SDA,gpio_get_level((gpio_num_t)MLX_MPU6050_SDA),
                    MLX_MPU6050_SCL,gpio_get_level((gpio_num_t)MLX_MPU6050_SCL));
            if (max_ready) {
                estimator.reset();
                oxygen.reset();
                last_ir=0;
                last_max_sample_us = esp_timer_get_time();
            }
        }
        if (max_ready) {
            Max30102Sample samples[MAX30102_FIFO_DEPTH];
            size_t count = 0;
            const esp_err_t err = max30102_read_fifo(MLX_MPU6050_I2C_PORT,
                                                      samples, MAX30102_FIFO_DEPTH, &count);
            if (err == ESP_OK) {
                if (count) {
                    last_max_sample_us = esp_timer_get_time();
                    max_samples_since_status += count;
                    for (size_t i = 0; i < count; ++i) {
                        last_ir=samples[i].ir;
                        last_red=samples[i].red;
                        if (last_ir<ir_min) ir_min=last_ir;
                        if (last_ir>ir_max) ir_max=last_ir;
                        ++window_samples;
                        estimator.add(last_ir);
                        oxygen.add(samples[i].red, last_ir);
                    }
                } else if (esp_timer_get_time() - last_max_sample_us > AUX_INTERVAL_US) {
                    max_ready = false; // FIFO detenido: volver a configurar.
                    estimator.reset();
                    oxygen.reset();
                    ESP_LOGW("NERA/MAX","FIFO sin muestras durante 1 s; reconfigurando");
                }
            } else {
                ++fifo_errors;
                if (now>=next_max_diagnostic_us)
                    ESP_LOGW("NERA/MAX","FIFO: %s; INVALID_STATE indica desbordamiento, se descarta BPM",
                             esp_err_to_name(err));
                estimator.reset();
                oxygen.reset();
                // El desbordamiento ya limpio el FIFO; un fallo I2C exige redeteccion.
                if (err != ESP_ERR_INVALID_STATE) max_ready = false;
            }
        }
        publish_heart(estimator, oxygen, max_ready,last_ir,
                      esp_timer_get_time()-last_max_sample_us<=AUX_INTERVAL_US,
                      last_max_sample_us);
        if (now>=next_max_diagnostic_us) {
            if (max_ready || window_samples || fifo_errors) {
                const char *state=!max_ready ? "sin conexion" : !window_samples ? "sin muestras FIFO" :
                    last_ir>=HeartRateEstimator::SATURATION_IR ? "IR saturado" :
                    last_ir<HeartRateEstimator::CONTACT_IR_MIN ? "IR bajo: revisar dedo/contacto" :
                    estimator.valid() ? "pulso valido" : "IR presente, buscando latidos coherentes";
                ESP_LOGI("NERA/MAX","PBA V2 OPTICA muestras=%u rojo=%lu IR=%lu rango=%lu..%lu erroresFIFO=%u BPM=%.0f SpO2est=%.0f R=%.2f picos=%u rechazados=%u edad=%lu ms retenido=%u; %s",
                    window_samples,(unsigned long)last_red,(unsigned long)last_ir,(unsigned long)(window_samples?ir_min:0),
                    (unsigned long)ir_max,fifo_errors,estimator.bpm(),oxygen.value(),oxygen.ratio(),
                    estimator.detected_peaks(),estimator.rejected_intervals(),
                    (unsigned long)(estimator.samples_since_valid()*10),estimator.holding()?1:0,state);
            }
            window_samples=fifo_errors=0; ir_min=UINT32_MAX; ir_max=0;
            next_max_diagnostic_us=now+MAX_DIAGNOSTIC_US;
        }
        if (!max_status_logged || max_ready != previous_max_ready) {
            if (max_ready) ESP_LOGI("NERA/SENSOR", "MAX30102 disponible; esperando pulso IR");
            else ESP_LOGW("NERA/SENSOR", "MAX30102 sin datos; reintento en 5 s");
            previous_max_ready = max_ready;
            max_status_logged = true;
        }

        if (now - last_aux_us < AUX_INTERVAL_US) {
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
            continue;
        }
        last_aux_us = now;
        esp_err_t mpu_result = ESP_OK;
        if (!mpu_ready) {
            mpu_result = mpu6050_init(MLX_MPU6050_I2C_PORT);
            mpu_ready = mpu_result == ESP_OK;
        }

        mlx90614_data_t mlx = {};
        const esp_err_t mlx_result = mlx90614_read(&mlx, MLX_MPU6050_I2C_PORT);
        publish_mlx(mlx, mlx_result);
        const bool mlx_valid = mlx_result == ESP_OK;
        if (!mlx_status_logged || mlx_valid != previous_mlx_valid) {
            if (mlx_valid) ESP_LOGI("NERA/SENSOR", "MLX90614 disponible");
            else ESP_LOGW("NERA/SENSOR", "MLX90614 sin lectura: %s", esp_err_to_name(mlx_result));
            previous_mlx_valid = mlx_valid;
            mlx_status_logged = true;
        }

        mpu6050_data_t mpu = {};
        if (mpu_ready) mpu_result = mpu6050_read(&mpu, MLX_MPU6050_I2C_PORT);
        publish_mpu(mpu, mpu_result);
        mpu_ready = mpu_result == ESP_OK;
        if (!mpu_status_logged || mpu_ready != previous_mpu_valid) {
            if (mpu_ready) ESP_LOGI("NERA/SENSOR", "MPU6050 disponible");
            else ESP_LOGW("NERA/SENSOR", "MPU6050 sin lectura: %s", esp_err_to_name(mpu_result));
            previous_mpu_valid = mpu_ready;
            mpu_status_logged = true;
        }
        if (now >= next_sensor_status_us) {
            const bool max_has_data = max_ready && max_samples_since_status > 0 &&
                                      esp_timer_get_time() - last_max_sample_us <= AUX_INTERVAL_US;
            ESP_LOGI("NERA/SENSOR", "ESTADO: MAX30102=%s | MLX90614=%s | MPU6050=%s",
                     max_has_data ? "RECIBE" : "SIN DATOS",
                     mlx_valid ? "RECIBE" : "SIN LECTURA",
                     mpu_ready ? "RECIBE" : "SIN LECTURA");
            if (mpu_ready)
                ESP_LOGI("NERA/SENSOR", "MPU6050 aceleracion X=%d Y=%d Z=%d; giro X=%d Y=%d Z=%d",
                         mpu.ax, mpu.ay, mpu.az, mpu.gx, mpu.gy, mpu.gz);
            max_samples_since_status = 0;
            next_sensor_status_us = now + SENSOR_STATUS_INTERVAL_US;
        }
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
    }
}
}

esp_err_t sensor_service_start(void) {
    if (started) return ESP_OK;
    snapshot_mutex = xSemaphoreCreateMutex();
    if (!snapshot_mutex) return ESP_ERR_NO_MEM;
    if (xTaskCreate(sensor_task, "nera_sensors", 4096, nullptr, 4, nullptr) != pdPASS) {
        vSemaphoreDelete(snapshot_mutex);
        snapshot_mutex = nullptr;
        return ESP_ERR_NO_MEM;
    }
    started = true;
    return ESP_OK;
}

esp_err_t sensor_service_get_snapshot(SensorSnapshot *snapshot) {
    if (!snapshot) return ESP_ERR_INVALID_ARG;
    if (!snapshot_mutex) return ESP_ERR_INVALID_STATE;
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(20)) != pdTRUE) return ESP_ERR_TIMEOUT;
    *snapshot = current;
    xSemaphoreGive(snapshot_mutex);
    // Nunca presentar un dato viejo como lectura actual.
    const int64_t now = esp_timer_get_time();
    if (now - snapshot->mlx_updated_us > MAX_SAMPLE_AGE_US) snapshot->mlx_valid = false;
    if (now - snapshot->mpu_updated_us > MAX_SAMPLE_AGE_US) snapshot->mpu_valid = false;
    if (now - snapshot->heart_updated_us > MAX_SAMPLE_AGE_US) snapshot->heart_valid = false;
    if (now - snapshot->spo2_updated_us > MAX_SAMPLE_AGE_US) snapshot->spo2_valid = false;
    return ESP_OK;
}
