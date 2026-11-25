/** Estimador experimental de pulso IR, 100 muestras/s.
 * Filtro PBA + mediana de intervalos medidos; la UI no lee el bus I2C. */
#pragma once
#include <stdint.h>
#include "pba_detector.h"
#include "pulse_intervals.h"

class HeartRateEstimator {
public:
    // MODIFICABLE: limites opticos y caducidad; no son umbrales medicos.
    static constexpr uint32_t CONTACT_IR_MIN=10000, SATURATION_IR=250000;
    static constexpr uint32_t HOLD_SAMPLES=300; // 3 s desde ultimo intervalo aceptado.
    static constexpr uint32_t WARMUP_SAMPLES=200;
    static constexpr uint32_t MIN_INTERVAL=30, MAX_INTERVAL=150; // 100 muestras/s.
    constexpr void reset() { *this=HeartRateEstimator{}; }

    constexpr void add(uint32_t ir) {
        // IMPORTANTE: retirar el dedo o saturar borra inmediatamente el BPM.
        if (ir<CONTACT_IR_MIN || ir>=SATURATION_IR) { reset(); return; }
        ++sample_index_;
        if (accepted_bpm_>0 && sample_index_-last_valid_sample_>HOLD_SAMPLES) {
            // IMPORTANTE: tras caducar, exigir intervalos nuevos; no reutilizar
            // el historial anterior para aceptar un unico pico al recuperar senal.
            accepted_bpm_=0;
            last_valid_sample_=last_peak_=0;
            intervals_.reset();
            rejected_since_valid_=false;
        }
        const bool beat=detector_.add(ir);
        if (sample_index_>=WARMUP_SAMPLES && beat) accept_peak();
    }
    constexpr bool valid() const {
        return accepted_bpm_>0 && last_valid_sample_ &&
               sample_index_-last_valid_sample_<=HOLD_SAMPLES;
    }
    constexpr float bpm() const { return valid() ? accepted_bpm_ : 0.f; }
    constexpr uint32_t samples_since_valid() const {
        return last_valid_sample_ ? sample_index_-last_valid_sample_ : HOLD_SAMPLES+1;
    }
    constexpr bool holding() const {
        return valid() && (rejected_since_valid_ ||
            float(samples_since_valid())>last_good_period_*1.5f);
    }
    constexpr unsigned detected_peaks() const { return detected_peaks_; }
    constexpr unsigned rejected_intervals() const { return rejected_intervals_; }
private:
    constexpr void accept_peak() {
        if (!last_peak_) { last_peak_=sample_index_; ++detected_peaks_; return; }
        const uint32_t period=sample_index_-last_peak_;
        if (period<MIN_INTERVAL) return; // Refractario: no desplazar la referencia.
        last_peak_=sample_index_;
        ++detected_peaks_;
        float measured=0;
        if (period>MAX_INTERVAL || !intervals_.add(period,&measured)) {
            ++rejected_intervals_;
            rejected_since_valid_=true;
            return; // No renovar la fecha ni prolongar indefinidamente un BPM.
        }
        accepted_bpm_=measured;
        last_valid_sample_=sample_index_;
        last_good_period_=6000.f/measured;
        rejected_since_valid_=false;
    }
    nera_sensor::PbaDetector detector_;
    nera_sensor::PulseIntervals intervals_;
    uint32_t sample_index_=0, last_peak_=0, last_valid_sample_=0;
    unsigned detected_peaks_=0, rejected_intervals_=0;
    float accepted_bpm_=0, last_good_period_=0;
    bool rejected_since_valid_=false;
};
