#pragma once
#include <stdint.h>

namespace nera_sensor {
// FUNCIONAL: valida intervalos REALES a 100 muestras/s. No interpola latidos.
// Tres de los ultimos cinco deben coincidir dentro del 20% de su mediana.
class PulseIntervals {
public:
    constexpr void reset() { *this = PulseIntervals{}; }
    constexpr unsigned consistent_count() const { return consistent_; }
    constexpr bool add(uint32_t gap, float *bpm) {
        consistent_ = 0;
        if (!bpm || gap == 0) return false;
        const float candidate = 6000.f / gap;
        if (candidate < 30.f || candidate > 220.f) return false;
        gaps_[next_] = gap;
        next_ = (next_ + 1) % 5;
        if (count_ < 5) ++count_;
        if (count_ < 3) return false;
        uint32_t sorted[5] = {};
        for (unsigned i = 0; i < count_; ++i) sorted[i] = gaps_[i];
        for (unsigned i = 1; i < count_; ++i) {
            const uint32_t value = sorted[i];
            unsigned j = i;
            while (j && sorted[j - 1] > value) { sorted[j] = sorted[j - 1]; --j; }
            sorted[j] = value;
        }
        const uint32_t median = sorted[count_ / 2];
        const uint32_t newest_delta = gap > median ? gap - median : median - gap;
        // Un intervalo atipico nunca renueva la fecha de la medicion.
        if (newest_delta * 5 > median) return false;
        uint32_t total = 0;
        for (unsigned i = 0; i < count_; ++i) {
            const uint32_t delta = gaps_[i] > median ? gaps_[i] - median : median - gaps_[i];
            if (delta * 5 <= median) { total += gaps_[i]; ++consistent_; }
        }
        if (consistent_ < 3) return false;
        *bpm = 6000.f * consistent_ / total;
        return true;
    }
private:
    uint32_t gaps_[5] = {};
    unsigned next_ = 0, count_ = 0, consistent_ = 0;
};
}
