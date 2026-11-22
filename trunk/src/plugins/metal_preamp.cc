// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "resources/faust.h"
// Use Guitarix's real-time data section for the circuit tables.
#undef __rt_data
#include "gx_plugin.h"
#include "gx_resampler.h"
#include "tamgamp/normalization.h"

namespace pluginlib { namespace metal_preamp {

using namespace faust;
#include <generated/ampsim/peavey_5150ii_lead.h>
#include <generated/ampsim/mesa_dual_rect_red.h>
#include <generated/ampsim/mesa_dc3_lead.h>

namespace {

constexpr int block_size = 256;
constexpr int circuit_rate = 96000;
using NormCurve = const float[32];

// Preserve the gain-dependent scaling applied by Tamgamp before the power amp.
float normalized_gain(float gain, NormCurve& curve)
{
    const int steps = sizeof(curve) / sizeof(curve[0]);
    const int index = int(gain * steps - 1);
    if (index <= 0) return curve[0];
    if (index >= steps - 1) return curve[steps - 1];
    const float fraction = gain - float(index) / steps;
    return curve[index] * (1 - fraction) + curve[index + 1] * fraction;
}

struct Knob {
    const char* label;
    const char* zone_name;
    float low;
    std::string parameter;
    float value = 0.5f;
    float* zone = nullptr;
};

class Preamp : public PluginDef, private UI {
    std::unique_ptr<dsp> circuit;
    NormCurve& norm;
    std::array<Knob,4> knobs;
    float* post_gain = nullptr;
    gx_resample::FixedRateResampler converter;
    std::vector<float> scratch;
    bool resample = false;

    void addVerticalSlider(const char* label, float* zone, float, float, float, float) override {
        if (std::strcmp(label, ".amp.postgain") == 0) post_gain = zone;
        for (auto& knob : knobs) {
            if (std::strcmp(label, knob.zone_name) == 0) knob.zone = zone;
        }
    }

    static int parameters(const ParamReg& reg) {
        auto& self = *static_cast<Preamp*>(reg.plugin);
        for (auto& k : self.knobs) {
            reg.registerFloatVar(k.parameter.c_str(), k.label, "S", "", &k.value,
                                 0.5f, k.low, 1.0f, 0.001f, nullptr);
        }
        return 0;
    }

    static int panel(const UiBuilder& ui, int form) {
        if (!(form & UI_FORM_STACK)) return -1;
        ui.openHorizontalBox("");
        for (const auto& k : static_cast<Preamp*>(ui.plugin)->knobs)
            ui.create_small_rackknob(k.parameter.c_str(), k.label);
        ui.closeBox();
        return 0;
    }

    static void prepare(unsigned rate, PluginDef* plugin) {
        auto& self = *static_cast<Preamp*>(plugin);
        self.resample = rate < circuit_rate && self.converter.setup(rate, circuit_rate) == 0;
        self.circuit->init(self.resample ? circuit_rate : rate);
        if (self.resample) self.scratch.resize(self.converter.max_out_count(block_size));
    }

    static void clear(PluginDef* plugin) {
        auto& self = *static_cast<Preamp*>(plugin);
        self.circuit->instanceClear();
        // Drain both resampler histories when reusing a bypassed head.
        if (self.resample) {
            float silence[block_size] = {};
            self.converter.up(block_size, silence, self.scratch.data());
            std::fill(self.scratch.begin(), self.scratch.end(), 0.f);
            self.converter.down(self.scratch.data(), silence);
        }
    }

    static void process(int count, float* input, float* output, PluginDef* plugin) {
        auto& self = *static_cast<Preamp*>(plugin);
        for (const auto& k : self.knobs) *k.zone = std::clamp(k.value, k.low, 1.f);
        *self.post_gain = normalized_gain(*self.knobs.front().zone, self.norm);
        if (!self.resample) {
            self.circuit->compute(count, &input, &output);
            return;
        }
        float* buf = self.scratch.data();
        while (count > 0) {
            const int frames = std::min(count, block_size);
            const int inner_frames = self.converter.up(frames, input, buf);
            self.circuit->compute(inner_frames, &buf, &buf);
            self.converter.down(buf, output);
            input += frames;
            output += frames;
            count -= frames;
        }
    }

public:
    Preamp(const char* key, const char* title, dsp_factory_t create, NormCurve& curve)
        : PluginDef(), circuit(create()), norm(curve), knobs{{
              {"Gain", ".amp.gain", 0.01f, std::string(key) + ".gain"},
              {"Bass", ".amp.bass", 0.f, std::string(key) + ".bass"},
              {"Mid", ".amp.middle", 0.f, std::string(key) + ".mid"},
              {"Treble", ".amp.treble", 0.f, std::string(key) + ".treble"}}} {
        circuit->buildUserInterface(this);
        id = key;
        name = title;
        category = "Tone Control";
        description = "Tamgamp high-gain preamp and tone stack";
        version = PLUGINDEF_VERSION;
        register_params = parameters;
        load_ui = panel;
        set_samplerate = prepare;
        clear_state = clear;
        activate_plugin = [](bool start, PluginDef* p) { if (start) clear(p); return 0; };
        mono_audio = process;
        delete_instance = [](PluginDef* p) { delete static_cast<Preamp*>(p); };
    }
};
} // namespace

PluginDef* meridian() {
    return new Preamp("meridian", "Meridian Lead", ampsim_peavey_5150ii_lead::instantiate, peavey_5150ii_lead_norm);
}
PluginDef* rectified() {
    return new Preamp("rectified", "Rectified Red", ampsim_mesa_dual_rect_red::instantiate, mesa_dual_rect_red_norm);
}
PluginDef* caliber() {
    return new Preamp("caliber", "Caliber Lead", ampsim_mesa_dc3_lead::instantiate, mesa_dc3_lead_norm);
}
} } // namespace pluginlib::metal_preamp
