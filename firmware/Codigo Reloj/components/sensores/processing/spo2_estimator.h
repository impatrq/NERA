/** Estimación óptica experimental, sin calibración clínica del conjunto.
 * Usa el cociente AC/DC rojo frente a IR de una ventana de cuatro segundos.
 * La ecuación 104-17R es el ejemplo de la guía MAX3010x de Analog Devices.
 * Un número solo se publica con contacto y dos ventanas concordantes. */
#pragma once
#include <math.h>
#include <stddef.h>
#include <stdint.h>

class SpO2Estimator {
public:
    static constexpr size_t WINDOW = 400; // 100 muestras/s, cuatro segundos.
    static constexpr size_t STEP = 100;

    void reset() {
        count_ = next_ = since_evaluation_ = 0;
        candidate_seen_ = valid_ = false;
        value_ = ratio_ = 0;
    }

    void add(uint32_t red, uint32_t ir) {
        if (red < 2000 || ir < 10000 || red >= 250000 || ir >= 250000) {
            reset();
            return;
        }
        red_[next_] = red;
        ir_[next_] = ir;
        next_ = (next_ + 1) % WINDOW;
        if (count_ < WINDOW) ++count_;
        if (count_ == WINDOW && ++since_evaluation_ >= STEP) {
            since_evaluation_ = 0;
            evaluate();
        }
    }

    bool valid() const { return valid_; }
    float value() const { return valid_ ? value_ : 0; }
    float ratio() const { return valid_ ? ratio_ : 0; }

private:
    void invalidate_window() {
        valid_ = false;
        candidate_seen_ = false;
    }

    void evaluate() {
        double red_sum = 0, ir_sum = 0, red_trend = 0, ir_trend = 0;
        constexpr double center = (WINDOW - 1) / 2.0;
        constexpr double x_sum_sq = WINDOW * (WINDOW * WINDOW - 1) / 12.0;
        for (size_t i = 0; i < WINDOW; ++i) {
            const size_t slot = (next_ + i) % WINDOW;
            red_sum += red_[slot];
            ir_sum += ir_[slot];
            red_trend += (double(i) - center) * red_[slot];
            ir_trend += (double(i) - center) * ir_[slot];
        }
        const double red_mean = red_sum / WINDOW, ir_mean = ir_sum / WINDOW;
        const double red_slope = red_trend / x_sum_sq;
        const double ir_slope = ir_trend / x_sum_sq;
        double red_var = 0, ir_var = 0, covariance = 0;
        for (size_t i = 0; i < WINDOW; ++i) {
            const size_t slot = (next_ + i) % WINDOW;
            const double red_ac = red_[slot] - red_mean - red_slope * (double(i) - center);
            const double ir_ac = ir_[slot] - ir_mean - ir_slope * (double(i) - center);
            red_var += red_ac * red_ac;
            ir_var += ir_ac * ir_ac;
            covariance += red_ac * ir_ac;
        }
        if (red_var <= 0 || ir_var <= 0) { invalidate_window(); return; }
        const double red_ac = sqrt(red_var / WINDOW);
        const double ir_ac = sqrt(ir_var / WINDOW);
        const double red_fraction = red_ac / red_mean;
        const double ir_fraction = ir_ac / ir_mean;
        const double correlation = covariance / sqrt(red_var * ir_var);
        if (red_fraction < 0.001 || ir_fraction < 0.001 ||
            red_fraction > 0.15 || ir_fraction > 0.15 || correlation < 0.7) {
            invalidate_window();
            return;
        }
        const double ratio = red_fraction / ir_fraction;
        const double estimate = 104.0 - 17.0 * ratio;
        if (!isfinite(estimate) || ratio < 0.2 || ratio > 1.8 ||
            estimate < 70 || estimate > 100) {
            invalidate_window();
            return;
        }
        valid_ = candidate_seen_ && fabs(estimate - last_candidate_) <= 3.0;
        candidate_seen_ = true;
        last_candidate_ = estimate;
        if (valid_) {
            value_ = static_cast<float>(estimate);
            ratio_ = static_cast<float>(ratio);
        }
    }

    uint32_t red_[WINDOW] = {}, ir_[WINDOW] = {};
    size_t count_ = 0, next_ = 0, since_evaluation_ = 0;
    bool candidate_seen_ = false, valid_ = false;
    double last_candidate_ = 0;
    float value_ = 0, ratio_ = 0;
};
