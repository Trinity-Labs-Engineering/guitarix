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
#include "faust-generated/spring_reverb.cc"
#include "faust-generated/plate_reverb.cc"
#include "faust-generated/hall_reverb.cc"
#include "faust-generated/ambient_reverb.cc"
#include "faust-generated/phase90.cc"
#include "faust-generated/klon_centaur.cc"
}
#undef min
#undef max

struct Control { float* value; float initial, low, high, step; };
std::map<std::string, Control> controls;
float* register_float(const char* id, const char*, const char*, const char*, float* value,
                      float initial, float low, float high, float step, const value_pair*) {
    *value = initial;
    controls[id] = {value, initial, low, high, step};
    return value;
}
int* register_int(const char*, const char*, const char*, const char*, int* value, int initial, int, int, const value_pair*) {
    *value = initial; return value;
}
bool* register_bool(const char*, const char*, const char*, const char*, bool* value, bool initial) {
    *value = initial; return value;
}
void prepare(PluginDef* p, int rate) {
    controls.clear();
    ParamReg reg {p, register_float, register_int, register_bool};
    p->register_params(reg);
    p->set_samplerate(rate, p);
    if (p->activate_plugin) p->activate_plugin(true, p);
    assert(p->mono_audio && !p->stereo_audio && p->clear_state);
    p->clear_state(p);
}
void set(const std::string& id, float value) { *controls.at(id).value = value; }
std::vector<float> render(PluginDef* p, const std::vector<float>& input, int block = 256, bool in_place = false) {
    auto output = in_place ? input : std::vector<float>(input.size());
    for (size_t i = 0; i < input.size(); i += block) {
        int n = std::min<size_t>(block, input.size() - i);
        p->mono_audio(n, in_place ? output.data() + i : const_cast<float*>(input.data()) + i,
                      output.data() + i, p);
    }
    for (float value : output) {
        if (!std::isfinite(value) || std::abs(value) >= 64) {
            std::cerr << p->id << " unstable output: " << value << '\n';
            std::abort();
        }
    }
    return output;
}
double energy(const std::vector<float>& samples, size_t begin = 0, size_t end = 0) {
    if (!end) end = samples.size();
    double result = 0;
    for (size_t i = begin; i < end; ++i) result += double(samples[i]) * samples[i];
    return result;
}
std::vector<float> guitar(int rate, int count) {
    std::vector<float> result(count);
    for (int i = 0; i < count; ++i)
        result[i] = 0.15 * std::sin(2 * M_PI * 110 * i / rate)
                  + 0.05 * std::sin(2 * M_PI * 330 * i / rate);
    return result;
}
void settle(PluginDef* p, int rate) { render(p, std::vector<float>(rate / 2)); }

void check_reverb(PluginDef* p, int rate) {
    const std::string mix = std::string(p->id) + ".mix";
    auto input = guitar(rate, rate / 5);
    set(mix, 0);
    settle(p, rate);
    auto dry = render(p, input);
    for (size_t i = 0; i < input.size(); ++i) assert(std::abs(dry[i] - input[i]) < 1e-6);

    p->clear_state(p);
    set(mix, 100);
    settle(p, rate);
    std::vector<float> impulse(rate * 12);
    impulse[0] = 0.25;
    auto tail = render(p, impulse);
    // True wet-only output: no copy of the input impulse at sample zero.
    assert(std::abs(tail[0]) < 1e-6);
    assert(energy(tail, rate / 10, rate * 2) > 1e-9);
    assert(energy(tail, rate * 10) < energy(tail, 0, rate * 2) * 0.05);

    p->clear_state(p);
    assert(energy(render(p, std::vector<float>(rate))) < 1e-20);
}

void check_processing(PluginDef* p, int rate) {
    auto input = guitar(rate, rate / 4 + 17);
    auto expected = render(p, input, 256);
    assert(energy(expected) > 1e-5);
    p->clear_state(p);
    auto actual = render(p, input, 37, true);
    for (size_t i = 0; i < input.size(); ++i) assert(std::abs(actual[i] - expected[i]) < 1e-6);

    // Change every exposed control independently, then all extremes together.
    for (const auto& [id, control] : controls) {
        for (float value : {control.low, control.high}) {
            *control.value = value;
            render(p, input);
        }
        *control.value = control.initial;
    }
    for (bool high : {false, true}) {
        for (const auto& [id, control] : controls) *control.value = high ? control.high : control.low;
        render(p, guitar(rate, rate * 2));
    }
    p->clear_state(p);
    auto silence = render(p, std::vector<float>(rate));
    assert(energy(silence) < 1e-15);
}

void check_decay_control(PluginDef* p) {
    const std::string id = p->id;
    const std::string decay = id + (id == "spring_reverb" ? ".dwell" : ".decay");
    double ratio[2];
    for (int high = 0; high < 2; ++high) {
        prepare(p, 48000);
        set(id + ".mix", 100);
        const auto control = controls.at(decay);
        *control.value = high ? control.high : control.low;
        settle(p, 48000);
        std::vector<float> impulse(48000 * 5);
        impulse[0] = 0.25;
        const auto tail = render(p, impulse);
        ratio[high] = energy(tail, 48000) / energy(tail, 0, 48000);
    }
    assert(ratio[1] > 10 * std::max(ratio[0], 1e-6));
}

int main(int argc, char**) {
    using Factory = PluginDef* (*)();
    const std::vector<Factory> factories = {
        effects::spring_reverb::plugin, effects::plate_reverb::plugin,
        effects::hall_reverb::plugin, effects::ambient_reverb::plugin,
        effects::phase90::plugin, effects::klon_centaur::plugin,
    };
    for (auto factory : factories) {
        auto* p = factory();
        prepare(p, 48000);
        if (argc > 1) {
            for (const auto& [id, c] : controls)
                std::cout << id << '\t' << c.initial << '\t' << c.low << '\t' << c.high << '\t' << c.step << '\n';
        } else {
            for (int rate : {44100, 48000, 96000, 192000}) {
                prepare(p, rate);
                check_processing(p, rate);
                prepare(p, rate);
                if (std::string(p->category) == "Reverb") check_reverb(p, rate);
                std::cout << p->id << " " << rate << " Hz passed" << std::endl;
            }
            if (std::string(p->category) == "Reverb") check_decay_control(p);
        }
        if (p->activate_plugin) p->activate_plugin(false, p);
        p->delete_instance(p);
    }
}
