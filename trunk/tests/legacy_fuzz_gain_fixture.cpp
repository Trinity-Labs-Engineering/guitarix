#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "gx_plugin.h"
#include "gx_faust_support.h"
#include "legacy_fuzz_plugins.h"
#undef min
#undef max

std::map<std::string, float*> controls;
float* register_float(const char* id, const char*, const char*, const char*, float* value,
                      float initial, float, float, float, const value_pair*) {
    *value = initial;
    controls[id] = value;
    return value;
}
int* register_int(const char*, const char*, const char*, const char*, int* value,
                  int initial, int, int, const value_pair*) {
    *value = initial;
    return value;
}
bool* register_bool(const char*, const char*, const char*, const char*, bool* value, bool initial) {
    *value = initial;
    return value;
}
void prepare(PluginDef* p, int rate) {
    controls.clear();
    ParamReg reg {p, register_float, register_int, register_bool};
    p->register_params(reg);
    p->set_samplerate(rate, p);
    if (p->activate_plugin) assert(p->activate_plugin(true, p) == 0);
    p->clear_state(p);
}
std::vector<float> signal(int rate, float amplitude) {
    std::vector<float> input(rate);
    for (int i = 0; i < rate; ++i)
        input[i] = amplitude * std::sin(2 * M_PI * 220 * i / rate);
    return input;
}
std::vector<float> render(PluginDef* p, const std::vector<float>& input, int block, bool in_place = false) {
    auto output = in_place ? input : std::vector<float>(input.size());
    for (size_t i = 0; i < input.size(); i += block) {
        int n = std::min<size_t>(block, input.size() - i);
        p->mono_audio(n, in_place ? output.data() + i : const_cast<float*>(input.data()) + i,
                      output.data() + i, p);
    }
    for (float value : output) {
        if (!std::isfinite(value)) {
            std::cerr << p->id << " produced non-finite audio\n";
            std::abort();
        }
    }
    return output;
}
double harmonic(const std::vector<float>& samples, int rate, int n) {
    double real = 0, imag = 0;
    for (int i = rate / 2; i < rate; ++i) {
        double angle = 2 * M_PI * 220 * n * i / rate;
        real += samples[i] * std::cos(angle);
        imag += samples[i] * std::sin(angle);
    }
    return std::hypot(real, imag);
}
void measure(PluginDef* p, int rate, float amplitude) {
    auto output = render(p, signal(rate, amplitude), 256);
    double power = 0;
    for (int i = rate / 2; i < rate; ++i) power += double(output[i]) * output[i];
    double fundamental = harmonic(output, rate, 1);
    std::cout << p->id << ' ' << rate << ' ' << amplitude << ' '
              << std::sqrt(power / (rate / 2)) << ' '
              << harmonic(output, rate, 3) / fundamental << ' '
              << harmonic(output, rate, 5) / fundamental << std::endl;
}
void check_mix(PluginDef* p, int rate) {
    auto mix = controls.find(std::string(p->id) + ".wet_dry");
    if (mix == controls.end()) return;
    auto input = signal(rate, 0.2);
    *mix->second = 0;
    const auto dry = render(p, input, 37, true);
    for (size_t i = 0; i < input.size(); ++i) {
        if (std::abs(dry[i] - input[i]) >= 1e-7) {
            std::cerr << p->id << " dry mix changed sample " << i << ": " << dry[i] << " vs " << input[i] << '\n';
            std::abort();
        }
    }

    // Mix must combine the compensated wet signal with unity-gain dry audio.
    prepare(p, rate);
    *controls.at(std::string(p->id) + ".wet_dry") = 100;
    const auto wet = render(p, input, 256);
    prepare(p, rate);
    *controls.at(std::string(p->id) + ".wet_dry") = 50;
    const auto mixed = render(p, input, 256);
    for (size_t i = 0; i < input.size(); ++i)
        assert(std::abs(mixed[i] - 0.5 * (input[i] + wet[i])) < 1e-6);
}
int main(int argc, char**) {
    using Factory = PluginDef* (*)();
    const std::vector<Factory> factories = {
        pluginlib::axface::plugin, pluginlib::bfuzz::plugin, pluginlib::ffreak::plugin,
        pluginlib::foxeylady::plugin, pluginlib::fumaster::plugin, pluginlib::fuzzdrive::plugin,
        fuzzface::plugin, pluginlib::fuzzfacefm::plugin, pluginlib::fuzzfacerm::plugin,
        pluginlib::ruin::plugin, pluginlib::susta::plugin, pluginlib::muff::plugin,
        pluginlib::bmp::plugin, pluginlib::buzz::plugin, pluginlib::hornet::plugin,
        pluginlib::scream::plugin, pluginlib::astrofuzz::plugin,
    };
    std::cout << std::setprecision(12);
    for (auto factory : factories) {
        for (int rate : {44100, 48000}) {
            for (float amplitude : {0.02f, 0.2f}) {
                auto* p = factory();
                prepare(p, rate);
                if (argc > 1) {
                    measure(p, rate, amplitude);
                } else {
                    const auto input = signal(rate, amplitude);
                    const auto expected = render(p, input, 256);
                    prepare(p, rate);
                    const auto actual = render(p, input, 37, true);
                    for (size_t i = 0; i < input.size(); ++i)
                        assert(std::abs(actual[i] - expected[i]) < 1e-6);
                    prepare(p, rate);
                    check_mix(p, rate);
                }
                p->delete_instance(p);
            }
        }
    }
}
