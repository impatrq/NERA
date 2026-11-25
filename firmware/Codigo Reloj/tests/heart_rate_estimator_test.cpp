// Pruebas de logica del detector: no simulan la placa ni la calidad optica.
#include "../components/sensores/processing/heart_rate_estimator.h"

constexpr bool detects_period(unsigned period, float expected_bpm) {
    HeartRateEstimator estimator;
    for (unsigned i = 0; i < 1800; ++i) {
        const unsigned phase = i % period;
        const int wave = phase < period / 2 ? (int)(phase * 2400 / period) :
                                              (int)((period - phase) * 2400 / period);
        estimator.add(70000 + wave);
    }
    return estimator.valid() && estimator.bpm() > expected_bpm - 3.0f &&
           estimator.bpm() < expected_bpm + 3.0f;
}
static_assert(detects_period(100, 60.0f));
static_assert(detects_period(67, 89.55f));
static_assert(detects_period(50, 120.0f));

constexpr bool rejects_bad_signal() {
    HeartRateEstimator estimator;
    for (unsigned i = 0; i < 1000; ++i)
        estimator.add(70000 + (i % 2 ? 5 : 0));
    if (estimator.valid()) return false;
    for (unsigned i = 0; i < 1800; ++i) {
        const unsigned phase = i % 100;
        const int wave = phase < 50 ? (int)(phase * 24) : (int)((100 - phase) * 24);
        estimator.add(70000 + wave);
    }
    if (!estimator.valid()) return false;
    estimator.add(0); // Se retiro el dedo.
    return !estimator.valid() && estimator.bpm() == 0.0f;
}
static_assert(rejects_bad_signal());
