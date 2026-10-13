/* Dual voice adapter for Guitarix's TONE3000-derived pitch shifter.
 * Copyright (C) 2026 Trinity Labs Engineering
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "gx_plugin.h"
#include "gx_poly_pitch_shifter.h"

namespace pluginlib {
namespace dual_pitch {

class DualPitch : public PluginDef {
    gx_engine::PolyphonicPitchShifter voices[2];
    float semitone1 = -12, semitone2 = 12;
    float level1 = 50, level2 = 50, dry = 0;

    static void init(unsigned int rate, PluginDef* plugin) {
        auto& self = *static_cast<DualPitch*>(plugin);
        // Both voices use the same 30 ms mode selected by Houston's mono pedal.
        // All allocation happens here, never in process/reset/activation.
        self.voices[0].prepare(rate, 1);
        self.voices[1].prepare(rate, 1);
    }

    static void reset(PluginDef* plugin) {
        auto& self = *static_cast<DualPitch*>(plugin);
        for (auto& voice : self.voices) voice.reset();
    }

    static int activate(bool, PluginDef* plugin) {
        reset(plugin);
        return 0;
    }

    static void process(int count, float* input, float* output, PluginDef* plugin) {
        auto& self = *static_cast<DualPitch*>(plugin);
        const double ratio1 = std::pow(2.0, self.semitone1 / 12.0);
        const double ratio2 = std::pow(2.0, self.semitone2 / 12.0);
        // Preserve the original input for both voices, including in-place calls.
        // Fixed scratch storage supports arbitrary host block sizes without allocation.
        float original[256], second[256];
        for (int offset = 0; offset < count; offset += 256) {
            const int size = std::min(256, count - offset);
            std::copy(input + offset, input + offset + size, original);
            self.voices[0].process(original, output + offset, size, ratio1,
                                  self.level1 * 0.02f, self.dry * 0.02f);
            self.voices[1].process(original, second, size, ratio2,
                                  self.level2 * 0.02f, 0.0f);
            for (int i = 0; i < size; ++i) output[offset + i] += second[i];
        }
    }

    static int register_parameters(const ParamReg& reg) {
        auto& self = *static_cast<DualPitch*>(reg.plugin);
        reg.registerFloatVar("dual_pitch.voice1", "Voice 1", "S", "Voice 1 pitch in semitones", &self.semitone1, -12, -12, 12, 1, 0);
        reg.registerFloatVar("dual_pitch.voice2", "Voice 2", "S", "Voice 2 pitch in semitones", &self.semitone2, 12, -12, 12, 1, 0);
        const char* trim = "Level: 0 = off, 50 = unity, 100 = +6 dB";
        reg.registerFloatVar("dual_pitch.level1", "V1 Level", "S", trim, &self.level1, 50, 0, 100, 1, 0);
        reg.registerFloatVar("dual_pitch.level2", "V2 Level", "S", trim, &self.level2, 50, 0, 100, 1, 0);
        reg.registerFloatVar("dual_pitch.dry", "Dry", "S", trim, &self.dry, 0, 0, 100, 1, 0);
        return 0;
    }

    static int ui(const UiBuilder& builder, int form) {
        if (!(form & UI_FORM_STACK)) return -1;
        builder.openHorizontalBox("");
        builder.create_small_rackknobr("dual_pitch.voice1", "Voice 1");
        builder.create_small_rackknobr("dual_pitch.voice2", "Voice 2");
        builder.create_small_rackknob("dual_pitch.level1", "V1 Level");
        builder.create_small_rackknob("dual_pitch.level2", "V2 Level");
        builder.create_small_rackknob("dual_pitch.dry", "Dry");
        builder.closeBox();
        return 0;
    }

    static void destroy(PluginDef* plugin) { delete static_cast<DualPitch*>(plugin); }

public:
    DualPitch() : PluginDef() {
        version = PLUGINDEF_VERSION;
        id = "dual_pitch";
        name = "Dual Pitch";
        category = "Modulation";
        description = "Two independent pitch-shifted voices with individual voice and dry levels";
        mono_audio = process;
        set_samplerate = init;
        activate_plugin = activate;
        clear_state = reset;
        register_params = register_parameters;
        load_ui = ui;
        delete_instance = destroy;
    }
};

PluginDef* plugin() { return new DualPitch; }

} // namespace dual_pitch
} // namespace pluginlib
