#pragma once

#include "esp_err.h"

// Prueba continua de banco, SOLO con las bobinas desconectadas.
// GPIO5 activo bajo: 2,5 ms bajos por ciclo de 15 Hz, para driver 2N2222.
esp_err_t gpio5_signal_test_start(void);
