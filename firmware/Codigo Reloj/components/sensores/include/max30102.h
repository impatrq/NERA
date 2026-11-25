/** MAX30102: entrega muestras rojo + IR; BPM usa IR. */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_err.h"

constexpr size_t MAX30102_FIFO_DEPTH = 32;
struct Max30102Sample { uint32_t red; uint32_t ir; };
esp_err_t max30102_init(i2c_port_t port);
// Extrae las muestras pendientes. Un desbordamiento limpia el FIFO y devuelve error.
esp_err_t max30102_read_fifo(i2c_port_t port, Max30102Sample *samples,
                             size_t capacity, size_t *sample_count);
