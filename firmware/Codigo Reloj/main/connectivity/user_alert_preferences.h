#pragma once
#include <stdint.h>
#include "esp_err.h"

namespace nera_preferences {
// MODIFICABLE: opciones de laboratorio; cambian la duracion, NO limites medicos.
// Cable BLE: version, avisos (0/1), sensibilidad (0..2), ejercicio (0/1).
struct Config { uint8_t version=1, enabled=1, sensitivity=0, exercise=0; };
constexpr bool valid(Config c) {
    return c.version==1 && c.enabled<=1 && c.sensitivity<=2 && c.exercise<=1;
}
constexpr int64_t hold_us(Config c) {
    return c.sensitivity==0 ? 5000000 : c.sensitivity==1 ? 10000000 : 20000000;
}
constexpr bool eligible(Config c, unsigned metric) {
    return c.enabled && !(c.exercise && metric==0);
}
// FUNCIONAL: lectura atomica. Una escritura se publica solo tras guardar NVS.
Config get();
bool saving();
esp_err_t init();
esp_err_t request(Config config);
}
