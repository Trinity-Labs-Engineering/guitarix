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
#include "faust-generated/thick_distortion.cc"
#include "faust-generated/bitdowner.cc"
#include "faust-generated/distortion2.cc"
#include "faust-generated/graphiceq.cc"
#include "faust-generated/selecteq.cc"
}
namespace pluginlib {
namespace bassboom { PluginDef* plugin(); }
namespace ts9sim { PluginDef* plugin(); }
}

static std::map<std::string, float*> controls;
static float* register_float(const char* id, const char*, const char*, const char*,
                             float* value, float initial, float, float, float,
                             const value_pair*) {
    controls[id] = value;
    *value = initial;
    return value;
}
static int* register_int(const char*, const char*, const char*, const char*,
                         int* value, int initial, int, int, const value_pair*) {
    *value = initial;
    return value;
}
static bool* register_bool(const char*, const char*, const char*, const char*,
                           bool* value, bool initial) {
    *value = initial;
    return value;
}
static void set(const std::string& id, float value) { *controls.at(id) = value; }
static double db(float value) { return std::pow(10.0, 0.05 * value); }
static void near(float actual, double expected, const char* context) {
    if (!std::isfinite(actual) || std::abs(actual - expected) > 2e-6) {
        std::cerr << context << ": actual=" << actual << " expected=" << expected << '\n';
        std::abort();
    }
}
static void check(PluginDef* plugin, bool thick, unsigned int rate) {
    controls.clear();
    ParamReg reg {plugin, register_float, register_int, register_bool};
    plugin->register_params(reg);
    plugin->set_samplerate(rate, plugin);
    const std::string prefix = thick ? "thick_distortion." : "bitdowner.";
    set(prefix + "input_gain", thick ? 15.0f : -5.0f);
    set(prefix + "volume", thick ? -2.5f : 1.0f);
    if (thick) set(prefix + "thickness", 0.125f);
    else {
        set(prefix + "bit_down", 8.5375f);
        set(prefix + "downsampling", 1.0f);
    }
    float input[32], output[32];
    for (int i = 0; i < 32; ++i) input[i] = (i % 7 < 3 ? 0.25f : -0.13f);
    // Current controls can be installed after init and before the first block.
    // The expected effect has its final gain from the first sample, not a
    // 20 ms control-envelope fade multiplied at both input and output.
    plugin->mono_audio(32, input, output, plugin);
    double history = 0.0;
    const double input_gain = db(*controls.at(prefix + "input_gain"));
    const double output_gain = db(*controls.at(prefix + "volume"));
    const double bits = thick ? 1.0 : std::pow(2.0, *controls.at(prefix + "bit_down") - 1.0);
    for (int i = 0; i < 32; ++i) {
        double value = input[i] * input_gain;
        if (!thick) value = std::floor(bits * value) / bits;
        value = std::fmax(-1.0, std::fmin(1.0, value));
        if (thick) { history = .125 * history + .875 * value; value = history; }
        near(output[i], value * output_gain, "initial target controls");
    }
    // Leave substantial audio/sample-hold history, then change controls while
    // bypassed. Reset must discard audio history and use these new controls.
    if (!thick) set(prefix + "downsampling", 8.0f);
    for (int i = 0; i < 20; ++i) plugin->mono_audio(32, input, output, plugin);
    set(prefix + "input_gain", -10.0f);
    set(prefix + "volume", -6.0f);
    plugin->clear_state(plugin);
    std::fill(input, input + 32, 0.0f);
    plugin->mono_audio(32, input, output, plugin);
    for (float value : output) near(value, 0.0, "cleared signal/sample-hold history");
    if (!thick) set(prefix + "downsampling", 1.0f);
    std::fill(input, input + 32, .1f);
    plugin->mono_audio(32, input, output, plugin);
    double reset_target = double(input[0]) * db(-10.0f);
    if (!thick) reset_target = std::floor(bits * reset_target) / bits;
    near(output[31], reset_target * db(-6.0f), "new controls on scene re-entry");
    for (int i = 0; i < 1500; ++i) plugin->mono_audio(32, input, output, plugin);
    const float before = output[31];
    // A live knob edit, without reset, must retain smoothing and not snap.
    set(prefix + "volume", 0.0f);
    plugin->mono_audio(32, input, output, plugin);
    assert(output[0] > before);
    assert(output[0] < before * 1.01f);
    for (int i = 0; i < 1500; ++i) plugin->mono_audio(32, input, output, plugin);
    near(output[31], before / db(-6.0f), "live control converges normally");
    plugin->delete_instance(plugin);
}

using Settings = std::map<std::string, float>;
static PluginDef* make(PluginDef* (*factory)(), const Settings& settings, unsigned int rate) {
    controls.clear();
    PluginDef* plugin = factory();
    ParamReg reg {plugin, register_float, register_int, register_bool};
    plugin->register_params(reg);
    plugin->set_samplerate(rate, plugin);
    for (const auto& item : settings) set(item.first, item.second);
    return plugin;
}
static void check_filter(PluginDef* (*factory)(), const Settings& settings,
                         unsigned int rate, bool resampled = false) {
    PluginDef* fresh = make(factory, settings, rate);
    PluginDef* settled = make(factory, settings, rate);
    float input[32] = {}, actual[32], expected[32];
    // Silence settles only controls. Filter states remain zero, so the same
    // subsequent waveform must have the same response from its first block.
    // 47040 samples also preserve the 44.1 -> 96k resampler's phase.
    for (int block = 0; block < 1470; ++block)
        settled->mono_audio(32, input, expected, settled);
    for (int i = 0; i < 32; ++i) input[i] = .17f * std::sin(i * .31f) + .03f;
    fresh->mono_audio(32, input, actual, fresh);
    settled->mono_audio(32, input, expected, settled);
    for (int i = 0; i < 32; ++i) near(actual[i], expected[i], fresh->id);
    if (!resampled) {
        for (int block = 0; block < 100; ++block)
            fresh->mono_audio(32, input, actual, fresh);
        fresh->clear_state(fresh);
        fresh->mono_audio(32, input, actual, fresh);
        for (int i = 0; i < 32; ++i) near(actual[i], expected[i], "filter history reset");
    }
    fresh->delete_instance(fresh);
    settled->delete_instance(settled);
}
int main() {
    for (unsigned int rate : {44100u, 48000u, 96000u}) {
        check(effects::thick_distortion::plugin(), true, rate);
        check(effects::bitdowner::plugin(), false, rate);
        check_filter(pluginlib::bassboom::plugin, {{"bboom.Drive", .75f}, {"bboom.Gain", -5.5f}}, rate);
        check_filter(pluginlib::ts9sim::plugin, {{"ts9sim.level", -5.0f}}, rate, true);
        check_filter(effects::distortion2::plugin, {{"distortion2.drive", 50.0f}, {"distortion2.gain", 5.0f}}, rate);
        check_filter(effects::graphiceq::plugin, {{"graphiceq.g1", -10.0f}, {"graphiceq.g11", -10.0f}}, rate);
        check_filter(effects::selecteq::plugin, {{"eqs.fs31_25", -15.0f}, {"eqs.fs16k", -15.0f}}, rate);
    }
}
