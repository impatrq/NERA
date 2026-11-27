#pragma once

#include "esp_err.h"
#include <stdbool.h>

// Inicializa apagado. Conserva los parametros de la prueba de banco existente.
// GPIO5 activo bajo: 2,5 ms bajos por ciclo de 15 Hz, para driver 2N2222.
esp_err_t gpio5_signal_test_start(void);
esp_err_t gpio5_signal_test_set_active(bool active);
