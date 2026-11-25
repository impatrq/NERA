#pragma once
#include "esp_err.h"

// FUNCIONAL: habilitar solo con CONFIG_NERA_ESPNOW_DIAGNOSTIC.
// Balizas y eventos de umbral de laboratorio con ACK; nunca ordena actuacion.
esp_err_t espnow_diagnostic_init(void);
