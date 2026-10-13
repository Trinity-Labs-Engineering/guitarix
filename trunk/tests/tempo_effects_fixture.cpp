#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "gx_plugin.h"
#include "gx_faust_support.h"
namespace effects {
#include "faust-generated/delay.cc"
#include "faust-generated/digital_delay.cc"
#include "faust-generated/duck_delay.cc"
#include "faust-generated/chorus_mono.cc"
#include "faust-generated/tremolo.cc"
#include "faust-generated/flanger_mono.cc"
#include "faust-generated/phaser_mono.cc"
#include "faust-generated/vibe_mono_lfo_sine.cc"
#include "faust-generated/vibe_lfo_sine.cc"
}
#undef min
#undef max
namespace pluginlib { namespace flanger_gx { PluginDef* plugin(); }
namespace reversedelay { PluginDef* plugin(); } }

struct Control { float* value; float low, high; };
std::map<std::string, Control> controls;
float* register_float(const char* id, const char*, const char*, const char*, float* value,
                      float initial, float low, float high, float, const value_pair*) {
    *value = initial;
    controls[id] = {value, low, high};
    return value;
}
int* register_int(const char*, const char*, const char*, const char*, int* value, int initial, int, int, const value_pair*) {
    *value = initial; return value;
}
bool* register_bool(const char*, const char*, const char*, const char*, bool* value, bool initial) {
    *value = initial; return value;
}
void set(const std::string& id, float value) { *controls.at(id).value = value; }
void prepare(PluginDef* p, int rate = 48000) {
    controls.clear();
    ParamReg reg {p, register_float, register_int, register_bool};
    p->register_params(reg);
    p->set_samplerate(rate, p);
    if (p->activate_plugin) p->activate_plugin(true, p);
    if (p->clear_state) p->clear_state(p);
}
void release(PluginDef* p) {
    if (p->activate_plugin) p->activate_plugin(false, p);
    p->delete_instance(p);
}
std::vector<float> render(PluginDef* p, const std::vector<float>& input) {
    std::vector<float> output(input.size());
    for (size_t offset = 0; offset < input.size(); offset += 256) {
        int count = std::min<size_t>(256, input.size() - offset);
        p->mono_audio(count, const_cast<float*>(input.data()) + offset, output.data() + offset, p);
    }
    for (float value : output) assert(std::isfinite(value));
    return output;
}
std::vector<float> impulse(PluginDef* p, int count) {
    render(p, std::vector<float>(48000)); // settle tempo interpolation and smooth controls
    std::vector<float> input(count);
    input[0] = 0.1f;
    return render(p, input);
}
float peak(const std::vector<float>& samples, int start, int end) {
    float result = 0;
    for (int i = start; i < end; ++i) result = std::max(result, std::abs(samples[i]));
    return result;
}
void delay_tail() {
    auto* p = effects::delay::plugin();
    for (float gain : {-10.f, 0.f, 10.f}) {
        prepare(p); set("delay.gain", gain);
        auto samples = impulse(p, 7 * 24000);
        float previous = peak(samples, 23980, 24020);
        const float linear = std::pow(10.f, gain / 20.f);
        const float ratio = linear / (1 + linear);
        for (int repeat = 2; repeat <= 6; ++repeat) {
            float current = peak(samples, repeat * 24000 - 20, repeat * 24000 + 20);
            assert(current > 0);
            assert(std::abs(current / previous - ratio) < 0.001);
            previous = current;
        }
    }
    release(p);
}
float digital_decay(float feedback, float level) {
    auto* p = effects::digital_delay::plugin(); prepare(p);
    set("dide.feedback", feedback); set("dide.level", level);
    auto samples = impulse(p, 7 * 24000);
    float a = peak(samples, 23980, 24200), b = peak(samples, 6 * 24000 - 20, 6 * 24000 + 200);
    assert(b > 0);
    release(p);
    return b / a;
}
void modulation(PluginDef* p, const std::string& bpm, const std::string& depth = "") {
    prepare(p);
    assert(controls.at(bpm).low == 24 && controls.at(bpm).high == 360);
    std::vector<float> input(48000);
    for (int i = 0; i < 48000; ++i) input[i] = 0.2 * std::sin(i * 0.057);
    if (!depth.empty()) {
        assert(controls.at(depth).low == 0 && controls.at(depth).high == 1);
        set(depth, 0);
        auto output = render(p, input);
        for (size_t i = 0; i < input.size(); ++i) assert(std::abs(output[i] - input[i]) < 1e-6);
        set(depth, 1);
    }
    for (float tempo : {24.f, 360.f}) { set(bpm, tempo); render(p, input); }
    release(p);
}
void delay_range(PluginDef* p, const std::string& bpm) {
    prepare(p, 192000);
    assert(controls.at(bpm).low == 24 && controls.at(bpm).high == 360);
    for (float tempo : {24.f, 360.f, 24.f}) {
        set(bpm, tempo);
        render(p, std::vector<float>(1100000, 0.01f));
    }
    release(p);
}
void vibe_rate() {
    ParamReg reg {nullptr, register_float, register_int, register_bool};
    effects::vibe_mono_lfo_sine::register_params(reg);
    effects::vibe_mono_lfo_sine::init(48000);
    effects::vibe_mono_lfo_sine::clear_state_f();
    assert(controls.at("univibe_mono.bpm").low == 24 && controls.at("univibe_mono.bpm").high == 360);
    set("univibe_mono.bpm", 120);
    // Vibe consumes one oscillator sample per 16 audio samples.
    std::vector<float> samples(3000);
    effects::vibe_mono_lfo_sine::compute(samples.size(), samples.data());
    int crossings = 0;
    for (size_t i = 1; i < samples.size(); ++i) if (samples[i-1] < .5 && samples[i] >= .5) ++crossings;
    assert(crossings == 1 || crossings == 2);
}
int main() {
    delay_tail();
    auto quiet = digital_decay(80, 20), loud = digital_decay(80, 90);
    assert(std::abs(quiet - loud) < 0.0001);
    assert(digital_decay(90, 50) > digital_decay(40, 50) * 10);
    modulation(effects::flanger_mono::plugin(), "flanger_mono.lfobpm", "flanger_mono.depth");
    modulation(effects::phaser_mono::plugin(), "phaser_mono.lfobpm", "phaser_mono.depth");
    modulation(effects::chorus_mono::plugin(), "chorus_mono.bpm");
    modulation(effects::tremolo::plugin(), "tremolo.bpm");
    modulation(pluginlib::flanger_gx::plugin(), "flanger_mono_gx.bpm");
    delay_range(effects::duck_delay::plugin(), "duckDelay.bpm");
    delay_range(pluginlib::reversedelay::plugin(), "reversedelay.bpm");
    vibe_rate();
    std::cout << "Tempo effects: feedback decay, depth, BPM ranges and buffers passed\n";
}
