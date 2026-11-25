#pragma once
#include <stdint.h>
// MODIFICABLE: parametros de laboratorio, NO limites clinicos; reglas independientes.
namespace nera_lab {
constexpr float HEART_ON=100.f, HEART_OFF=90.f, TEMP_ON=33.f, TEMP_OFF=31.f;
constexpr int64_t HOLD_US=5000000, FRESH_US=3000000;
class Rule {
public:
    constexpr Rule(float on, float off) : on_(on), off_(off) {}
    // 0 sin evento, 1 activa, 2 fin por histeresis, 3 fin por dato invalido.
    // FUNCIONAL: sample_us es la fecha REAL de la muestra, no del sondeo.
    constexpr int update(bool valid, float value, int64_t sample_us, int64_t hold_us=HOLD_US) {
        if (!valid) { since_=-1; if (active_) { active_=false; return 3; } return 0; }
        if (active_) {
            if (value<off_) { active_=false; since_=-1; return 2; }
            return 0;
        }
        if (value<=on_) { since_=-1; return 0; }
        if (since_<0 || sample_us<since_) since_=sample_us;
        if (sample_us-since_>=hold_us) { active_=true; return 1; }
        return 0;
    }
    // FUNCIONAL: al cambiar el perfil, no arrastrar una duracion parcial anterior.
    constexpr void reset_qualification() { since_=-1; }
private:
    float on_, off_;
    int64_t since_=-1;
    bool active_=false;
};
}
