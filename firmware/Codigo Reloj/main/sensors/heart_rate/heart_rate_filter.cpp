#include "heart_rate_filter.h"

#include <math.h>

void HeartRateFilter::reset()
{
    sample_count_ = 0;
    next_sample_ = 0;
    for (float &sample : samples_) {
        sample = 0.0f;
    }
}

bool HeartRateFilter::add_sample(float bpm, float *filtered_bpm)
{
    if (filtered_bpm == nullptr || !isfinite(bpm) ||
        bpm < kMinimumBpm || bpm > kMaximumBpm) {
        return false;
    }

    samples_[next_sample_] = bpm;
    next_sample_ = (next_sample_ + 1) % kWindowSize;
    if (sample_count_ < kWindowSize) {
        ++sample_count_;
    }

    float total = 0.0f;
    for (size_t index = 0; index < sample_count_; ++index) {
        total += samples_[index];
    }

    *filtered_bpm = total / static_cast<float>(sample_count_);
    return true;
}

bool HeartRateFilter::has_value() const
{
    return sample_count_ > 0;
}
