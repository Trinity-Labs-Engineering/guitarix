// Real amp voicing DSP must not share controls or history with matrix pedals.
#include <cassert>
#include <cmath>
#include <map>
#include <string>
#include "gx_plugin.h"
enum { PGNI_MATRIX_SPLITTER = 0x800000, PGNI_MATRIX_COMBINER = 0x1000000 };
#include "gx_matrix.h"
#include "gx_faust_support.h"
#define _(text) (text)
namespace gx_engine { namespace gx_effects {
#include "selecteq.cc"
} }
namespace pluginlib { namespace jcm800pre { PluginDef* plugin(); } }

static std::map<std::string, float*> parameters;
static float* reg_float(const char* id, const char*, const char*, const char*,
    float* value, float initial, float, float, float, const value_pair*) {
    assert(parameters.emplace(id, value).second);
    *value = initial;
    return value;
}

static void check(PluginDef* (*factory)(), const char* prefix, const char* control,
                  float amp_value, float pedal_value, unsigned int sample_rate) {
    using gx_engine::SecondPluginInstance;
    PluginDef* pedal = factory();
    SecondPluginInstance second(factory());
    SecondPluginInstance amp(factory(), prefix);
    SecondPluginInstance reference(factory(), "reference");
    parameters.clear();
    for (auto* dsp : {pedal, static_cast<PluginDef*>(&second),
                     static_cast<PluginDef*>(&amp), static_cast<PluginDef*>(&reference)}) {
        ParamReg reg{}; reg.plugin = dsp; reg.registerFloatVar = reg_float;
        assert(dsp->register_params(reg) == 0);
        dsp->set_samplerate(sample_rate, dsp);
    }
    const std::string base(pedal->id);
    *parameters.at(base + "." + control) = pedal_value;
    *parameters.at(base + "__2." + control) = pedal_value / 2;
    *parameters.at(std::string(prefix) + "." + control) = amp_value;
    *parameters.at(std::string("reference.") + control) = amp_value;
    assert(parameters.at(base + "." + control) != parameters.at(std::string(prefix) + "." + control));
    assert(parameters.at(base + "__2." + control) != parameters.at(std::string(prefix) + "." + control));
    double energy = 0;
    for (int block = 0; block < 32; ++block) {
        float input[128], other[128], output[128], expected[128], discarded[128];
        for (int i = 0; i < 128; ++i) {
            input[i] = .1f * std::sin((128 * block + i) * .071f);
            other[i] = .2f * std::cos((128 * block + i) * .173f);
        }
        // Changing/processing either pedal must not disturb the amp's history.
        if (block == 16) *parameters.at(base + "." + control) = amp_value;
        pedal->mono_audio(128, other, discarded, pedal);
        amp.mono_audio(128, input, output, &amp);
        second.mono_audio(128, other, discarded, &second);
        reference.mono_audio(128, input, expected, &reference);
        for (int i = 0; i < 128; ++i) {
            assert(std::isfinite(output[i]));
            assert(output[i] == expected[i]);
            energy += output[i] * output[i];
        }
    }
    assert(energy > 0.001);
    pedal->delete_instance(pedal);
}

int main() {
    for (unsigned int rate : {48000u, 96000u}) {
        check(gx_engine::gx_effects::selecteq::plugin, "amp_eqs", "fs1k", 6, -12, rate);
        check(pluginlib::jcm800pre::plugin, "amp_JCM800Pre", "P6v", .6f, .2f, rate);
    }
}
