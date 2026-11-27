#pragma once
#include "esp_err.h"

// FUNCIONAL: habilitar solo con CONFIG_NERA_ESPNOW_DIAGNOSTIC.
// Umbral -> recepcion en app por BLE -> orden ESP-NOW -> ACK de GPIO5.
esp_err_t espnow_diagnostic_init(void);
