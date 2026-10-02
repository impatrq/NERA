#pragma once

#include <stddef.h>

class HeartRateFilter {
public:
    static constexpr float kMinimumBpm = 30.0f;
    static constexpr float kMaximumBpm = 220.0f;
    static constexpr size_t kWindowSize = 5;

    void reset();
    bool add_sample(float bpm, float *filtered_bpm);
    bool has_value() const;

private:
    float samples_[kWindowSize] = {};
    size_t sample_count_ = 0;
    size_t next_sample_ = 0;
};
