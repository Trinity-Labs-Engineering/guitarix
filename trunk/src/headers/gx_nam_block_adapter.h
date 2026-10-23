#pragma once

#include <algorithm>

namespace gx_engine {

// NAM history/working storage scales with this size, not the JACK callback
// size. Split model-rate audio after resampling so arbitrary host block sizes
// and sample-rate ratios cannot grow each resident graph's working buffers.
constexpr int kNamModelBlockFrames = 256;
constexpr int kMaxNamHostBlockFrames = 8192;

template <typename Model, typename Sample>
inline void process_nam_mono_bounded(Model* model, Sample* input, Sample* output, int frames) {
    for (int offset = 0; offset < frames; offset += kNamModelBlockFrames) {
        const int count = std::min<int>(kNamModelBlockFrames, frames - offset);
        Sample* inputs[] = {input + offset};
        Sample* outputs[] = {output + offset};
        model->process(inputs, outputs, count);
    }
}

} // namespace gx_engine
