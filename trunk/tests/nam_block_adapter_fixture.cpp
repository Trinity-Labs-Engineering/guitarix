#include <cassert>
#include <iostream>
#include <vector>

#include "gx_nam_block_adapter.h"

using gx_engine::kMaxNamHostBlockFrames;
using gx_engine::kNamModelBlockFrames;

struct Model {
    int samples = 0;
    void process(float** input, float** output, int count) {
        assert(count > 0 && count <= kNamModelBlockFrames);
        for (int i = 0; i < count; ++i)
            output[0][i] = input[0][i] + static_cast<float>(samples++);
    }
};
struct PluginDef {};
struct NeuralAmp : PluginDef {
    Model model;
    void compute(int count, float* input, float* output) {
        assert(count > 0 && count <= kMaxNamHostBlockFrames);
        gx_engine::process_nam_mono_bounded(&model, input, output, count);
    }
    static void compute_static(int, float*, float*, PluginDef*);
};
struct NeuralAmpMulti : NeuralAmp {
    static void compute_static(int, float*, float*, PluginDef*);
};
#include "nam_callbacks_under_test.h"

template <typename Wrapper>
void verify(bool inplace) {
    Wrapper wrapper;
    int processed = 0;
    for (int count : {0, 1, 32, 255, 256, 257, 8192, 8193, 32769, 17}) {
        std::vector<float> input(count + 2, 123.0f), output(count + 2, 123.0f);
        for (int i = 0; i < count; ++i) input[i + 1] = static_cast<float>(i % 7);
        float* target = inplace ? input.data() + 1 : output.data() + 1;
        Wrapper::compute_static(count, input.data() + 1, target, &wrapper);
        for (int i = 0; i < count; ++i)
            assert(target[i] == static_cast<float>(i % 7 + processed + i));
        assert(input.front() == 123.0f && input.back() == 123.0f);
        assert(output.front() == 123.0f && output.back() == 123.0f);
        processed += count;
        assert(wrapper.model.samples == processed);
    }
}

int main() {
    verify<NeuralAmp>(false);
    verify<NeuralAmp>(true);
    verify<NeuralAmpMulti>(false);
    verify<NeuralAmpMulti>(true);
    std::cout << "nam-blocks-ok\n";
}
