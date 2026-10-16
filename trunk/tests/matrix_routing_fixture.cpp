// Exercise the production mono routing loop and real generated DSP factories.
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "gx_plugin.h"

enum {
    PGNI_RESIDENT_TRACK_BYPASS = 0x400000,
    PGNI_MATRIX_SPLITTER = 0x800000,
    PGNI_MATRIX_COMBINER = 0x1000000,
};
#include "gx_matrix.h"

#define N_(value) value
#define _(value) value
#define FAUSTFLOAT float
#define always_inline
#include "gain.cc"
#include "../src/plugins/dual_pitch.cc"
#include "gx_vibe_lfo.h"

using namespace gx_engine;
struct Plugin {
    bool audible = true;
    int lane = 0;
    bool source = false;
    PluginDef* definition = nullptr;
    int position = 0;
    bool resident = false;
    enum { POST_WEIGHT = 2000 };
    bool get_rt_on_off() { return audible; }
    int get_rt_matrix_lane() { return lane; }
    bool get_rt_matrix_source() { return source; }
    PluginDef* get_pdef() { return definition; }
    bool get_on_off() { return audible; }
    bool get_scene_resident() { return resident; }
    int get_matrix_lane() { return lane; }
    bool get_matrix_source() { return source; }
    int get_position() { return position; }
    int get_effect_post_pre() { return 1; }
};
class PluginList {
public:
    std::map<std::string, Plugin*> pmap;
    bool validate_matrix(const std::map<std::string, int>& values, std::string& error) const;
};
#include "matrix_validation_under_test.h"
struct monochain_data {
    process_mono_audio func = nullptr;
    PluginDef* plugin = nullptr;
    Plugin* owner = nullptr;
};
class MonoModuleChain {
public:
    enum RampMode { ramp_mode_down_dead, ramp_mode_off };
    RampMode ramp = ramp_mode_off;
    std::vector<monochain_data> nodes;
    RampMode get_ramp_mode() { return ramp; }
    monochain_data* get_rt_chain() { return nodes.data(); }
    void process(int count, float* input, float* output);
};
#include "matrix_process_under_test.h"

static void twice(int n, float* in, float* out, PluginDef*) {
    for (int i = 0; i < n; ++i) out[i] = 2.0f * in[i];
}
static void thrice(int n, float* in, float* out, PluginDef*) {
    for (int i = 0; i < n; ++i) out[i] = 3.0f * in[i];
}
static void stereo(int n, float* a, float* b, float* x, float* y, PluginDef*) {
    for (int i = 0; i < n; ++i) { x[i] = a[i] * 2; y[i] = b[i] * 4; }
}
static std::map<std::string, float*> parameters;
static float* reg_float(const char* id, const char*, const char*, const char*,
                        float* value, float initial, float, float, float, const value_pair*) {
    assert(!parameters.count(id));
    parameters[id] = value;
    *value = initial;
    return value;
}
static void expect(const float* values, float expected, int count = 32) {
    for (int i = 0; i < count; ++i) {
        if (std::abs(values[i] - expected) >= 0.0001f)
            std::cerr << "sample " << i << ": expected " << expected << ", received " << values[i] << '\n';
        assert(std::abs(values[i] - expected) < 0.0001f);
    }
}

int main() {
    MatrixBoundary split(true), combine(false);
    PluginDef double_gain{}, triple_gain{}, stereo_effect{};
    double_gain.mono_audio = twice;
    triple_gain.mono_audio = thrice;
    stereo_effect.stereo_audio = stereo;
    Plugin upper{true, 1}, lower{true, 2}, serial, source{true, 0, true};
    Plugin split_node{true, 0, false, &split, 1};
    Plugin combine_node{true, 0, false, &combine, 11};
    Plugin branch{true, 1, false, &double_gain, 2};
    double_gain.id = "gain";
    stereo_effect.id = "chorus";
    Plugin source_node{false, 0, false, &stereo_effect, 2};
    PluginList plugin_list;
    plugin_list.pmap = {{"matrix_splitter", &split_node},
                        {"matrix_combiner", &combine_node},
                        {"gain", &branch}, {"chorus", &source_node}};
    std::string error;
    assert(plugin_list.validate_matrix({}, error));
    assert(!plugin_list.validate_matrix({{"matrix_combiner.on_off", 0}}, error));
    assert(!plugin_list.validate_matrix({{"chorus.matrix_source", 1}}, error));
    assert(!plugin_list.validate_matrix({{"gain.matrix_lane", 0}}, error));
    assert(!plugin_list.validate_matrix({{"gain.position", 12}}, error));
    assert(plugin_list.validate_matrix({{"matrix_splitter.on_off", 0},
                                       {"chorus.matrix_source", 1},
                                       {"gain.position", 4}}, error));
    assert(!plugin_list.validate_matrix({{"matrix_splitter.on_off", 0},
                                        {"chorus.matrix_source", 1},
                                        {"chorus.matrix_lane", 1}}, error));
    assert(plugin_list.validate_matrix({{"matrix_splitter.position", 11},
                                       {"gain.on_off", 0}}, error));
    MonoModuleChain chain;
    float input[32], output[32];
    std::fill(input, input + 32, 1.0f);
    chain.nodes = {{split.mono_audio, &split, &serial},
                   {twice, &double_gain, &upper},
                   {thrice, &triple_gain, &lower},
                   {combine.mono_audio, &combine, &serial},
                   {twice, &double_gain, &serial}, {}};
    chain.process(32, input, output);
    expect(output, 5.0f); // (2 + 3) / 2 then the unchanged mono amp stage.
    split.balance = -1;
    split.clear_state(&split);
    chain.process(32, input, output);
    expect(output, 2.0f);
    split.balance = 1;
    split.clear_state(&split);
    chain.process(32, input, output);
    expect(output, 3.0f);
    split.balance = 0;
    split.clear_state(&split);
    combine.balance = -1;
    combine.clear_state(&combine);
    chain.process(32, input, output);
    expect(output, 4.0f);
    combine.balance = 1;
    combine.clear_state(&combine);
    chain.process(32, input, output);
    expect(output, 6.0f);
    combine.balance = 0;
    combine.clear_state(&combine);
    lower.audible = false;
    chain.process(32, input, output);
    expect(output, 3.0f);
    lower.audible = true;

    // A stereo source supplies distinct lanes and preserves the graph in bypass.
    chain.nodes[0] = {MatrixBoundary::passthrough, &stereo_effect, &source};
    chain.process(32, input, output);
    expect(output, 16.0f);
    source.audible = false;
    chain.process(32, input, output);
    expect(output, 5.0f);

    // Legacy malformed graphs still combine before reaching a serial amp.
    chain.nodes.erase(chain.nodes.begin() + 3);
    chain.process(32, input, output);
    expect(output, 5.0f);
    chain.nodes = {{split.mono_audio, &split, &serial},
                   {combine.mono_audio, &combine, &serial}, {}};
    chain.process(32, input, output);
    expect(output, 1.0f);
    chain.ramp = MonoModuleChain::ramp_mode_down_dead;
    chain.process(32, input, output);
    expect(output, 0.0f);

    // HAE's 10 ms linear balance ramp reaches the endpoint after 480 samples.
    split.set_samplerate(48000, &split);
    split.balance = 1;
    std::vector<float> ramp_input(480, 1), ramp_lower(480);
    split.split_audio(ramp_input.data(), ramp_lower.data(), 480);
    assert(ramp_input.front() < 1.0f && ramp_input.front() > 0.99f);
    assert(std::abs(ramp_input.back()) < 0.000001f);
    expect(ramp_lower.data(), 1.0f, 480);

    // Real Faust gain instances own independent controls and smoother history.
    PluginDef* first = gain::plugin();
    SecondPluginInstance second(gain::plugin());
    ParamReg reg{}; reg.registerFloatVar = reg_float;
    reg.plugin = first; first->register_params(reg);
    reg.plugin = &second; second.register_params(reg);
    assert(parameters.at("gain.gain") != parameters.at("gain__2.gain"));
    *parameters.at("gain.gain") = 0;
    *parameters.at("gain__2.gain") = 6.0205999f;
    first->set_samplerate(48000, first);
    second.set_samplerate(48000, &second);
    std::vector<float> ones(20000, 1), a(20000), b(20000);
    first->mono_audio(20000, ones.data(), a.data(), first);
    second.mono_audio(20000, ones.data(), b.data(), &second);
    assert(std::abs(a.back() - 1) < 0.0001f);
    assert(std::abs(b.back() - 2) < 0.0001f);
    first->clear_state(first);
    second.mono_audio(32, input, output, &second);
    expect(output, 2.0f);
    first->delete_instance(first);

    // Two actual dual-pitch pedals (four voices) retain separate delay history.
    PluginDef* pitch = pluginlib::dual_pitch::plugin();
    SecondPluginInstance pitch_second(pluginlib::dual_pitch::plugin());
    SecondPluginInstance pitch_reference(pluginlib::dual_pitch::plugin(), "pitch_reference");
    for (PluginDef* effect : {pitch, static_cast<PluginDef*>(&pitch_second),
                             static_cast<PluginDef*>(&pitch_reference)}) {
        reg.plugin = effect;
        effect->register_params(reg);
        effect->set_samplerate(48000, effect);
    }
    *parameters.at("dual_pitch.voice1") = -12;
    *parameters.at("dual_pitch__2.voice1") = 12;
    *parameters.at("pitch_reference.voice1") = 12;
    float sine[128], shifted[128], reference[128], discarded[128];
    for (int block = 0; block < 128; ++block) {
        for (int i = 0; i < 128; ++i)
            sine[i] = std::sin((block * 128 + i) * 440.0f * 6.2831853f / 48000);
        if (block == 64) pitch->clear_state(pitch);
        pitch->mono_audio(128, sine, discarded, pitch);
        pitch_second.mono_audio(128, sine, shifted, &pitch_second);
        pitch_reference.mono_audio(128, sine, reference, &pitch_reference);
        for (int i = 0; i < 128; ++i) {
            assert(std::isfinite(shifted[i]));
            assert(shifted[i] == reference[i]);
        }
    }
    pitch->delete_instance(pitch);

    // Univibe's former namespace-global oscillator is now per DSP object.
    VibeMonoLfo lfo_a, lfo_b, lfo_reference;
    reg.plugin = nullptr;
    lfo_a.register_params(reg);
    {
        ScopedParamNamespace names(reg, nullptr, "univibe_mono__2");
        lfo_b.register_params(names.reg);
    }
    {
        ScopedParamNamespace names(reg, nullptr, "vibe_reference");
        lfo_reference.register_params(names.reg);
    }
    lfo_a.init(48000); lfo_b.init(48000); lfo_reference.init(48000);
    lfo_b.compute(128, shifted); lfo_reference.compute(128, reference);
    lfo_a.compute(128, discarded); lfo_a.clear_state_f();
    lfo_b.compute(128, shifted); lfo_reference.compute(128, reference);
    for (int i = 0; i < 128; ++i) assert(shifted[i] == reference[i]);
    std::cout << "matrix-routing-ok\n";
}
