/* NAM silence-bias compensation. GPL-2.0-or-later. */
#pragma once
#include <algorithm>
#include <cmath>

namespace gx_engine {

// Control thread only, immediately after ResetAndPrewarm. Measure the model's
// equilibrium output, not the live guitar signal. Unstable/non-finite output
// is not a DC offset and must not be subtracted from subsequent audio.
template<class Model>
float measure_nam_dc(Model* model) {
    constexpr int count = 512;
    float silence[count]{}, output[count];
    float* inputs[] = {silence};
    float* outputs[] = {output};
    model->process(inputs, outputs, count);
    double sum = 0;
    float low = output[0], high = output[0];
    for (float sample : output) {
        if (!std::isfinite(sample)) return 0;
        low = std::min(low, sample);
        high = std::max(high, sample);
        sum += sample;
    }
    return high - low <= 1e-6f ? static_cast<float>(sum / count) : 0;
}

inline void remove_nam_dc(float* output, int count, float offset) {
    // No filter history, ramp, or added latency. Remove the model's constant
    // bias before resampling, output gain, and downstream amp/IR processing.
    for (int i = 0; i < count; ++i) output[i] -= offset;
}

} // namespace gx_engine
