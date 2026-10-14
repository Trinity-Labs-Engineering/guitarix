/* Copyright (C) 2026 Trinity Labs Engineering
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once
#include "gx_plugin.h"
#include "gx_harmoniser.h"
#include "gx_poly_pitch_shifter.h"
#include <string>

namespace gx_engine {

template<bool Dual> class Harmoniser : public PluginDef {
    harmony::FundamentalTracker tracker;
    PolyphonicPitchShifter voices[Dual ? 2 : 1];
    float tonic = 3, mode = 0, interval1 = 2, interval2 = 4;
    float wet = 50, mix = 50, dry = 50;
    float gate[2] = {0, 0}, gate_step = 1.0f / 240;

    static void init(unsigned int rate, PluginDef* plugin) {
        auto& self = *static_cast<Harmoniser*>(plugin);
        self.tracker.prepare(rate);
        for (auto& voice : self.voices) voice.prepare(rate, 1);
        self.gate_step = 1.0f / std::max(1.0f, rate * 0.005f);
        reset(plugin);
    }
    static void reset(PluginDef* plugin) {
        auto& self = *static_cast<Harmoniser*>(plugin);
        self.tracker.reset();
        for (auto& voice : self.voices) voice.reset();
        self.gate[0] = self.gate[1] = 0;
    }
    static int activate(bool, PluginDef* plugin) { reset(plugin); return 0; }
    static int discrete(float value, int low, int high) {
        return std::isfinite(value) ? static_cast<int>(std::lround(std::max(float(low), std::min(float(high), value)))) : low;
    }
    static float level(float value) { return std::isfinite(value) ? std::max(0.0f, std::min(100.0f, value)) : 0; }

    static void process(int count, float* input, float* output, PluginDef* plugin) {
        auto& self = *static_cast<Harmoniser*>(plugin);
        const int root = discrete(self.tonic, 0, 11), scale = discrete(self.mode, 0, 6);
        const int intervals[] = {discrete(self.interval1, -7, 7), discrete(self.interval2, -7, 7)};
        const float balance = level(self.mix) * 0.01f;
        const float gains[] = {Dual ? balance : level(self.wet) * 0.02f, 1 - balance};
        const float dry_gain = level(self.dry) * 0.02f;
        float original[256], shifted[256];
        for (int offset = 0; offset < count;) {
            // Split on detector boundaries so results do not depend on JACK's
            // block size. Preserve the input for in-place host processing.
            const int size = std::min(std::min(256, count - offset), self.tracker.samples_until_analysis());
            std::copy(input + offset, input + offset + size, original);
            const int note = self.tracker.note();
            for (int i = 0; i < size; ++i) output[offset + i] = dry_gain * original[i];
            for (int voice = 0; voice < (Dual ? 2 : 1); ++voice) {
                const int interval = intervals[voice];
                const bool voiced = note >= 0 || interval == 0 || std::abs(interval) == 7;
                const double ratio = std::pow(2.0, harmony::interval_shift(note, root, scale, interval) / 12.0);
                self.voices[voice].process(original, shifted, size, ratio, gains[voice], 0);
                for (int i = 0; i < size; ++i) {
                    self.gate[voice] = voiced ? std::min(1.0f, self.gate[voice] + self.gate_step)
                                              : std::max(0.0f, self.gate[voice] - self.gate_step);
                    output[offset + i] += shifted[i] * self.gate[voice];
                }
            }
            self.tracker.push(original, size); // once, shared by both voices
            offset += size;
        }
    }
    static int register_parameters(const ParamReg& reg) {
        auto& self = *static_cast<Harmoniser*>(reg.plugin);
        const std::string prefix = Dual ? "dual_harmoniser." : "harmoniser.";
        const auto add = [&](const char* key, const char* name, const char* help, float* value,
                             float initial, float low, float high, const value_pair* labels = nullptr) {
            reg.registerFloatVar((prefix + key).c_str(), name, "S", help, value, initial, low, high, 1, labels);
        };
        static const value_pair tonics[] = {{"A", "A"}, {"A#", "A#"}, {"B", "B"}, {"C", "C"},
            {"C#", "C#"}, {"D", "D"}, {"D#", "D#"}, {"E", "E"}, {"F", "F"}, {"F#", "F#"},
            {"G", "G"}, {"G#", "G#"}, {nullptr, nullptr}};
        static const value_pair modes[] = {{"Major", "Major"}, {"Dorian", "Dorian"}, {"Phrygian", "Phrygian"},
            {"Lydian", "Lydian"}, {"Mixolydian", "Mixolydian"}, {"Minor", "Minor"}, {"Locrian", "Locrian"}, {nullptr, nullptr}};
        add("tonic", "Tonic", "Key tonic (A = 0)", &self.tonic, 3, 0, 11, tonics);
        add("mode", "Mode", "Musical mode", &self.mode, 0, 0, 6, modes);
        const char* interval_help = "Diatonic steps: -7 = octave down, 0 = unison, +7 = octave up; out-of-mode notes shift an octave";
        add(Dual ? "voice1" : "interval", Dual ? "Voice 1 Interval" : "Interval", interval_help, &self.interval1, 2, -7, 7);
        if (Dual) {
            add("voice2", "Voice 2 Interval", interval_help, &self.interval2, 4, -7, 7);
            add("mix", "Voice Mix", "0 = only Voice 2, 100 = only Voice 1", &self.mix, 50, 0, 100);
        } else add("wet", "Wet", "0 = off, 50 = unity, 100 = +6 dB", &self.wet, 50, 0, 100);
        add("dry", "Dry", "0 = off, 50 = unity, 100 = +6 dB", &self.dry, 50, 0, 100);
        return 0;
    }
    static int ui(const UiBuilder& builder, int form) {
        if (!(form & UI_FORM_STACK)) return -1;
        const std::string prefix = Dual ? "dual_harmoniser." : "harmoniser.";
        builder.openHorizontalBox("");
        builder.create_selector((prefix + "tonic").c_str(), "Tonic");
        builder.create_selector((prefix + "mode").c_str(), "Mode");
        builder.create_small_rackknobr((prefix + (Dual ? "voice1" : "interval")).c_str(), "Interval");
        if (Dual) builder.create_small_rackknobr((prefix + "voice2").c_str(), "Voice 2");
        builder.create_small_rackknob((prefix + (Dual ? "mix" : "wet")).c_str(), Dual ? "Voice Mix" : "Wet");
        builder.create_small_rackknob((prefix + "dry").c_str(), "Dry");
        builder.closeBox();
        return 0;
    }
    static void destroy(PluginDef* plugin) { delete static_cast<Harmoniser*>(plugin); }
public:
    Harmoniser() : PluginDef() {
        version = PLUGINDEF_VERSION;
        id = Dual ? "dual_harmoniser" : "harmoniser";
        name = Dual ? "Dual Harmoniser" : "Harmoniser";
        category = "Modulation";
        description = "Key-based harmony following the lowest detected fundamental";
        mono_audio = process; set_samplerate = init; activate_plugin = activate;
        clear_state = reset; register_params = register_parameters; load_ui = ui; delete_instance = destroy;
    }
};
} // namespace gx_engine
