#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

typedef struct {
    int hr;
    int spo2;
    float temp;
    float ax;
    float ay;
    float az;
    bool ble_conectado;
} nera_metricas_reloj_t;

esp_err_t nera_pantalla_principal_mostrar(void);
void nera_pantalla_principal_actualizar(const nera_metricas_reloj_t *metricas);
