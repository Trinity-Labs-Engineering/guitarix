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
#include "faust-generated/echo.cc"
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

struct Control { float* value; float low, high, initial, step; };
std::map<std::string, Control> controls;
float* register_float(const char* id, const char*, const char*, const char*, float* value,
                      float initial, float low, float high, float step, const value_pair*) {
    *value = initial;
    controls[id] = {value, low, high, initial, step};
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
std::vector<float> render(PluginDef* p, const std::vector<float>& input, int block = 256) {
    std::vector<float> output(input.size());
    for (size_t offset = 0; offset < input.size(); offset += block) {
        int count = std::min<size_t>(block, input.size() - offset);
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
// Test the actual Houston filter defaults with guitar-frequency audio, not an
// impulse that can hide the loss of fundamentals inside the feedback filters.
void digital_catalog_tail(float highpass, float lowpass) {
    auto* p = effects::digital_delay::plugin(); prepare(p);
    set("dide.highpass", highpass); set("dide.howpass", lowpass);
    set("dide.feedback", 90); set("dide.bpm", 120);
    render(p, std::vector<float>(48000));
    std::vector<float> input(7 * 24000);
    for (int i = 0; i < 2400; ++i) {
        const float envelope = std::pow(std::sin(3.141592653589793 * i / 2400), 2);
        input[i] = .2f * envelope * std::sin(2 * 3.141592653589793 * 440 * i / 48000);
    }
    auto output = render(p, input);
    const float first = peak(output, 24000, 27000);
    const float sixth = peak(output, 6 * 24000, 6 * 24000 + 3000);
    assert(first > .09 && sixth / first > .5);
    release(p);
}
void modulation(PluginDef* p, const std::string& bpm, const std::string& depth = "") {
    prepare(p);
    assert(controls.at(bpm).low > 0 && controls.at(bpm).low <= 24 && controls.at(bpm).high == 360);
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
    assert(controls.at(bpm).low == 24 && controls.at(bpm).high == 6000);
    for (float tempo : {24.f, 6000.f, 24.f}) {
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
// Measure actual first-repeat times, not just expanded slider metadata.
void slapback(PluginDef* p) {
    const std::string bpm = std::string(p->id) + ".bpm";
    for (int rate : {44100, 48000, 96000, 192000}) {
        prepare(p, rate);
        if (rate == 44100) {
            const auto c = controls.at(bpm);
            std::cout << "META\t" << bpm << '\t' << c.initial << '\t' << c.low << '\t' << c.high << '\t' << c.step << '\n';
        }
        for (int ms : {10, 40, 80, 100, 120, 2500}) {
            set(bpm, 60000.f / ms);
            if (p->clear_state) p->clear_state(p);
            const int expected = std::lround(ms * rate / 1000.0);
            if (std::string(p->id) == "reversedelay") {
                // Reverse delay is chunk-based; verify its real cycle length
                // and finite window output even at the shortest intervals.
                for (float window : {0.f, .5f, 100.f}) {
                    set("reversedelay.window", window); p->clear_state(p);
                    auto y = render(p, std::vector<float>(expected + 1, .1f), 32);
                    assert(std::abs(*controls.at("reversedelay.buf_indication").value) < .01);
                    for (float v : y) assert(std::abs(v) <= .11);
                }
            } else {
                if (std::string(p->id) == "echo") set("echo.percent", 50);
                render(p, std::vector<float>(rate * 2), 32);
                std::vector<float> x(expected + 32); x[0] = .1f;
                const auto y = render(p, x, 32);
                int first = 1;
                while (first < int(y.size()) && std::abs(y[first]) < .0001f) ++first;
                if (std::abs(first - expected) > 3) {
                    std::cerr << p->id << " at " << rate << " Hz, " << ms << " ms: first echo " << first << " expected " << expected << '\n';
                    std::abort();
                }
                assert(peak(y, std::max(1, expected - 3), expected + 4) > .02);
            }
            // Resident reset must not replay an old short delay or tail.
            if (p->clear_state) p->clear_state(p);
            assert(peak(render(p, std::vector<float>(rate / 10), 32), 0, rate / 10) == 0);
        }
    }
    release(p);
}
int main(int argc, char** argv) {
    assert(argc == 3);
    digital_catalog_tail(std::stof(argv[1]), std::stof(argv[2]));
    delay_tail();
    auto quiet = digital_decay(80, 20), loud = digital_decay(80, 90);
    assert(std::abs(quiet - loud) < 0.0001);
    assert(digital_decay(90, 50) > digital_decay(40, 50) * 10);
    modulation(effects::flanger_mono::plugin(), "flanger_mono.lfobpm"); // zero sweep retains a static comb
    modulation(effects::phaser_mono::plugin(), "phaser_mono.lfobpm", "phaser_mono.depth");
    modulation(effects::chorus_mono::plugin(), "chorus_mono.bpm");
    modulation(effects::tremolo::plugin(), "tremolo.bpm");
    modulation(pluginlib::flanger_gx::plugin(), "flanger_mono_gx.bpm");
    delay_range(effects::duck_delay::plugin(), "duckDelay.bpm");
    delay_range(pluginlib::reversedelay::plugin(), "reversedelay.bpm");
    vibe_rate();
    slapback(effects::delay::plugin());
    slapback(effects::echo::plugin());
    slapback(effects::digital_delay::plugin());
    slapback(effects::duck_delay::plugin());
    slapback(pluginlib::reversedelay::plugin());
    std::cout << "Tempo effects: feedback decay, depth, BPM ranges and buffers passed\n";
}
