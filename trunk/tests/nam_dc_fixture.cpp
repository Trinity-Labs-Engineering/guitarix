#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#ifdef GX_TEST_FAUST_MACROS
#include "gx_faust_support.h"
#endif
#include "gx_nam_dc.h"
#ifdef GX_TEST_FAUST_MACROS
static_assert(min(2, 3) == 2 && max(2, 3) == 3);
#endif

struct Model {
    float bias = -.001426122f;
    bool unstable = false, invalid = false;
    void process(float** input, float** output, int count) {
        for (int i = 0; i < count; ++i)
            output[0][i] = invalid ? std::numeric_limits<float>::quiet_NaN() :
                std::tanh(2 * input[0][i]) + bias + (unstable && i % 2 ? .001f : 0);
    }
};

int main() {
    Model model;
    const float offset = gx_engine::measure_nam_dc(&model);
    assert(offset == model.bias);
    float input[32], output[32];
    float* inputs[] = {input};
    float* outputs[] = {output};
    for (int i = 0; i < 32; ++i) input[i] = std::sin(i * .1f);
    model.process(inputs, outputs, 32);
    gx_engine::remove_nam_dc(output, 32, offset);
    for (int i = 0; i < 32; ++i) assert(std::fabs(output[i] - std::tanh(2 * input[i])) < 1e-7f);

    // A pedal's silence offset excites a shared high-pass stage on every
    // bypass edge, outlasting a 4 ms output envelope. Removing the offset
    // upstream prevents this ringing without delaying the switch.
    double old_raw = 0, raw_state = 0, old_fixed = 0, fixed_state = 0;
    double raw_peak = 0, fixed_peak = 0;
    for (int i = 0; i < 48000; ++i) {
        const bool on = (i / 4800) % 2;
        const double raw = on ? model.bias : 0;
        const double fixed = on ? model.bias - offset : 0;
        raw_state = raw - old_raw + .999 * raw_state;
        fixed_state = fixed - old_fixed + .999 * fixed_state;
        old_raw = raw; old_fixed = fixed;
        if (i % 4800 >= 192) {
            raw_peak = (std::max)(raw_peak, std::fabs(raw_state));
            fixed_peak = (std::max)(fixed_peak, std::fabs(fixed_state));
        }
    }
    assert(raw_peak > .001 && fixed_peak == 0);
    model.unstable = true;
    assert(gx_engine::measure_nam_dc(&model) == 0);
    model.invalid = true;
    assert(gx_engine::measure_nam_dc(&model) == 0);
}
