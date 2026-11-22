/* Whole-chord transposition for Guitarix / Houston.
 * Copyright (C) 2026 Trinity Labs Engineering
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "gx_plugin.h"
#include "gx_poly_pitch_shifter.h"

namespace pluginlib {
namespace virtual_capo {

class VirtualCapo : public PluginDef {
    gx_engine::PolyphonicPitchShifter voice;
    float semitone = 2, bass_mode = 0;

    static void init(unsigned int rate, PluginDef* plugin) {
        // More landing range for chords whose strings share no short period.
        // The existing pitch effects retain their faster 30 ms setting.
        static_cast<VirtualCapo*>(plugin)->voice.prepare(rate, 0);
    }
    static void reset(PluginDef* plugin) {
        static_cast<VirtualCapo*>(plugin)->voice.reset();
    }
    static int activate(bool, PluginDef* plugin) { reset(plugin); return 0; }
    static void process(int count, float* input, float* output, PluginDef* plugin) {
        auto& self = *static_cast<VirtualCapo*>(plugin);
        self.voice.set_bass_mode(self.bass_mode >= 0.5f);
        const double semitones = std::isfinite(self.semitone)
            ? std::round(std::max(-12.0f, std::min(12.0f, self.semitone))) : 0.0;
        self.voice.process(input, output, count, std::pow(2.0, semitones / 12.0), 1.0f, 0.0f);
    }
    static int register_parameters(const ParamReg& reg) {
        auto& self = *static_cast<VirtualCapo*>(reg.plugin);
        reg.registerFloatVar("virtual_capo.semitone", "Transpose", "S", "Shift every string by this many semitones", &self.semitone, 2, -12, 12, 1, 0);
        static const value_pair instruments[] = {{"Guitar", "Guitar"}, {"Bass", "Bass"}, {nullptr, nullptr}};
        reg.registerFloatVar("virtual_capo.bass_mode", "Instrument", "S", "Bass allows extra range for low notes", &self.bass_mode, 0, 0, 1, 1, instruments);
        return 0;
    }
    static int ui(const UiBuilder& builder, int form) {
        if (!(form & UI_FORM_STACK)) return -1;
        builder.openHorizontalBox("");
        builder.create_small_rackknobr("virtual_capo.semitone", "Transpose");
        builder.create_selector("virtual_capo.bass_mode", "Instrument");
        builder.closeBox();
        return 0;
    }
    static void destroy(PluginDef* plugin) { delete static_cast<VirtualCapo*>(plugin); }
public:
    VirtualCapo() : PluginDef() {
        version = PLUGINDEF_VERSION;
        id = "virtual_capo";
        name = "Virtual Capo";
        category = "Modulation";
        description = "Whole-chord transposition with extra sustain stability and a longer delay";
        mono_audio = process;
        set_samplerate = init;
        activate_plugin = activate;
        clear_state = reset;
        register_params = register_parameters;
        load_ui = ui;
        delete_instance = destroy;
    }
};

PluginDef* plugin() { return new VirtualCapo; }

} // namespace virtual_capo
} // namespace pluginlib
