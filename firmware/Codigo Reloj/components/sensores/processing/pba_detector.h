/* Adapted from SparkFun MAX3010x heartRate.cpp (Maxim PBA algorithm).
 * https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/blob/master/src/heartRate.cpp
 * NERA changes: instance state/reset, 64-bit DC/FIR arithmetic for 18-bit input,
 * no Arduino dependency. This is an experimental estimator, not a diagnosis.
 *
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 */
#pragma once
#include <stdint.h>

namespace nera_sensor {
class PbaDetector {
public:
    constexpr void reset() { *this = PbaDetector{}; }
    constexpr bool add(uint32_t sample) {
        if (!primed_) { dc_ = (int64_t)sample << 15; primed_ = true; }
        dc_ += (((int64_t)sample << 15) - dc_) / 16;
        const int32_t previous = signal_;
        signal_ = filter((int32_t)sample - (int32_t)(dc_ >> 15));
        bool beat = false;
        if (previous < 0 && signal_ >= 0) {
            const int32_t amplitude = maximum_ - minimum_;
            // MODIFICABLE: amplitud AC maxima relativa al nivel DC (5%).
            // Evita el techo fijo de 1000 que descartaba pulsaciones grandes.
            const int32_t relative_limit = (int32_t)(dc_ >> 15) / 20;
            const int32_t limit = relative_limit > 1000 ? relative_limit : 1000;
            beat = amplitude > 20 && amplitude < limit;
            positive_ = true;
            negative_ = false;
            maximum_ = 0;
        }
        if (previous > 0 && signal_ <= 0) {
            positive_ = false;
            negative_ = true;
            minimum_ = 0;
        }
        if (positive_ && signal_ > previous) maximum_ = signal_;
        if (negative_ && signal_ < previous) minimum_ = signal_;
        return beat;
    }
private:
    static constexpr uint16_t coefficients[12] =
        {172,321,579,927,1360,1858,2390,2916,3391,3768,4012,4096};
    constexpr int32_t filter(int32_t input) {
        samples_[offset_] = input;
        int64_t sum = (int64_t)coefficients[11] * samples_[(offset_ + 21) & 31];
        for (unsigned i = 0; i < 11; ++i)
            sum += (int64_t)coefficients[i] *
                (samples_[(offset_ + 32 - i) & 31] + samples_[(offset_ + 10 + i) & 31]);
        offset_ = (offset_ + 1) & 31;
        return (int32_t)(sum >> 15);
    }
    int64_t dc_ = 0;
    int32_t signal_ = 0, minimum_ = 0, maximum_ = 0;
    int32_t samples_[32] = {};
    unsigned offset_ = 0;
    bool primed_ = false, positive_ = false, negative_ = false;
};
}
