#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <new>
#include <string>
#include <vector>
#include "gx_plugin.h"

static size_t allocations = 0;
void* operator new(size_t size) { ++allocations; if (void* p = std::malloc(size)) return p; throw std::bad_alloc(); }
void* operator new[](size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
namespace pluginlib { namespace virtual_capo { PluginDef* plugin(); } }

std::map<std::string, float*> controls;
float* reg_float(const char* id, const char*, const char*, const char*, float* value,
                 float initial, float low, float high, float step, const value_pair* choices) {
    controls[id] = value; *value = initial;
    const bool pitch = std::string(id) == "virtual_capo.semitone";
    assert(low == (pitch ? -12 : 0) && high == (pitch ? 12 : 1) && step == 1);
    assert(initial == (pitch ? 2 : 0));
    assert(pitch || (choices && std::string(choices[0].value_id) == "Guitar"));
    return value;
}

std::vector<float> run(PluginDef* p, const std::vector<float>& input, bool inplace, int block) {
    p->clear_state(p);
    std::vector<float> output = input;
    const size_t before = allocations;
    for (int pos = 0; pos < int(input.size()); pos += block) {
        const int count = std::min(block, int(input.size()) - pos);
        p->mono_audio(count, inplace ? output.data() + pos : const_cast<float*>(input.data()) + pos, output.data() + pos, p);
    }
    assert(allocations == before);
    for (float sample : output) assert(std::isfinite(sample));
    return output;
}

int main(int argc, char** argv) {
    auto* p = pluginlib::virtual_capo::plugin();
    ParamReg reg {p, reg_float, nullptr, nullptr};
    assert(p->register_params(reg) == 0 && controls.size() == 2);
    assert(std::string(p->id) == "virtual_capo");
    if (argc == 4) {
        p->set_samplerate(std::atoi(argv[1]), p);
        *controls.at("virtual_capo.semitone") = std::atof(argv[2]);
        *controls.at("virtual_capo.bass_mode") = std::atof(argv[3]);
        std::vector<float> input;
        float chunk[4096];
        while (std::cin.read(reinterpret_cast<char*>(chunk), sizeof(chunk)) || std::cin.gcount())
            input.insert(input.end(), chunk, chunk + std::cin.gcount() / sizeof(float));
        const auto output = run(p, input, false, 16);
        std::cout.write(reinterpret_cast<const char*>(output.data()), output.size() * sizeof(float));
    } else {
        for (int rate : {44100, 48000, 96000}) {
            p->set_samplerate(rate, p);
            std::vector<float> input(rate);
            for (int i = 0; i < rate; ++i) input[i] = .2f * std::sin(6.283185307179586 * 110 * i / rate);
            for (int bass : {0, 1}) for (float semitone : {-12.f, -2.f, 0.f, 2.f, 12.f}) {
                *controls.at("virtual_capo.semitone") = semitone;
                *controls.at("virtual_capo.bass_mode") = bass;
                const auto expected = run(p, input, false, 16);
                for (int block : {1, 127, 4097}) {
                    const auto output = run(p, input, true, block);
                    for (int i = 0; i < rate; ++i) assert(std::abs(output[i] - expected[i]) < 2e-6f);
                }
                const size_t before = allocations;
                p->clear_state(p); p->activate_plugin(false, p); p->activate_plugin(true, p);
                p->mono_audio(0, nullptr, nullptr, p);
                assert(allocations == before);
            }
            // Silent scene re-entry must not replay the preceding held chord.
            std::fill(input.begin(), input.end(), 0);
            for (float value : run(p, input, true, 16)) assert(value == 0);
        }
        std::cout << "virtual-capo-ok\n";
    }
    p->delete_instance(p);
}
