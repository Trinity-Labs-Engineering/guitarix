/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include "gx_plugin.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace gx_engine {
namespace artemis_eq {
constexpr unsigned band_count = 16;
struct Settings {
    float enabled = 0, frequency = 1000, gain = 0, q = 1, shape = 0;
};
inline double bounded(double value, double low, double high, double fallback) {
    return std::isfinite(value) ? std::max(low, std::min(high, value)) : fallback;
}
struct Coefficients {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    bool flat = true;
    bool operator==(const Coefficients& other) const {
        return b0 == other.b0 && b1 == other.b1 && b2 == other.b2
            && a1 == other.a1 && a2 == other.a2 && flat == other.flat;
    }
};
// RBJ biquads: https://www.w3.org/TR/audio-eq-cookbook/
// Keep this contract in sync with waveforms/src/artemisEq.ts.
inline Coefficients coefficients(const Settings& s, double sample_rate) {
    const double gain = bounded(s.gain, -24, 24, 0);
    const int shape = int(std::round(bounded(s.shape, 0, 4, 0)));
    // Cuts have no gain control: zero dB must not bypass a high/low-pass.
    if (!std::isfinite(s.enabled) || s.enabled < 0.5f || (gain == 0 && shape < 3)) return {};
    const double rate = bounded(sample_rate, 8000, 384000, 48000);
    const double frequency = bounded(s.frequency, 20, std::min(20000.0, rate * 0.49), 1000);
    const double q = bounded(s.q, 0.1, 18, 1);
    const double w = 6.283185307179586 * frequency / rate;
    const double c = std::cos(w), a = std::pow(10, gain / 40);
    double alpha = std::sin(w) / (2 * q);
    double b0, b1, b2, a0, a1, a2;
    if (shape == 0) {
        b0 = 1 + alpha * a; b1 = -2 * c; b2 = 1 - alpha * a;
        a0 = 1 + alpha / a; a1 = -2 * c; a2 = 1 - alpha / a;
    } else if (shape >= 3) {
        // Two-pole (12 dB/octave) cuts. Q controls the cutoff resonance.
        b0 = b2 = (shape == 3 ? 1 + c : 1 - c) / 2;
        b1 = (shape == 3 ? -2 : 2) * b0;
        a0 = 1 + alpha; a1 = -2 * c; a2 = 1 - alpha;
    } else {
        // Q controls shelf steepness, without resonant overshoot. S stays
        // strictly positive and below one even at the control limits.
        const double slope = q / (q + 0.5);
        alpha = std::sin(w) * 0.5 * std::sqrt((a + 1 / a) * (1 / slope - 1) + 2);
        const double r = 2 * std::sqrt(a) * alpha, sum = a + 1, diff = a - 1;
        if (shape == 1) {
            b0 = a * (sum - diff * c + r); b1 = 2 * a * (diff - sum * c); b2 = a * (sum - diff * c - r);
            a0 = sum + diff * c + r; a1 = -2 * (diff + sum * c); a2 = sum + diff * c - r;
        } else {
            b0 = a * (sum + diff * c + r); b1 = -2 * a * (diff + sum * c); b2 = a * (sum + diff * c - r);
            a0 = sum - diff * c + r; a1 = 2 * (diff - sum * c); a2 = sum - diff * c - r;
        }
    }
    return {b0/a0, b1/a0, b2/a0, a1/a0, a2/a0, false};
}
struct Filter {
    Coefficients c;
    double z1 = 0, z2 = 0;
    double tick(double input) {
        if (c.flat) return input;
        const double output = c.b0 * input + z1;
        z1 = c.b1 * input - c.a1 * output + z2;
        z2 = c.b2 * input - c.a2 * output;
        if (!std::isfinite(output) || !std::isfinite(z1) || !std::isfinite(z2)) {
            z1 = z2 = 0; return input;
        }
        if (std::abs(z1) < 1e-30) z1 = 0;
        if (std::abs(z2) < 1e-30) z2 = 0;
        return output;
    }
};
struct Band {
    Filter current, next;
    int remaining = 0;
    void prepare(const Coefficients& target, bool initial, int fade) {
        if (initial) { current = {target, 0, 0}; remaining = 0; }
        else if (!remaining && !(target == current.c)) {
            next = {target, 0, 0}; remaining = fade;
        }
    }
    double tick(double input, int fade) {
        const double old = current.tick(input);
        if (!remaining) return old;
        const double fresh = next.tick(input);
        const double amount = double(fade - remaining + 1) / fade;
        if (--remaining == 0) current = next;
        return old + (fresh - old) * amount;
    }
};
} // namespace artemis_eq

class ArtemisEq : public PluginDef {
    std::array<artemis_eq::Settings, artemis_eq::band_count> settings{};
    std::array<artemis_eq::Band, artemis_eq::band_count> bands{};
    double rate = 48000;
    int fade = 240;
    bool initial = true;
    static void reset(PluginDef* p) {
        auto& self = *static_cast<ArtemisEq*>(p);
        self.bands = {}; self.initial = true;
    }
    static int activate(bool, PluginDef* p) { reset(p); return 0; }
    static void init(unsigned rate, PluginDef* p) {
        auto& self = *static_cast<ArtemisEq*>(p);
        self.rate = artemis_eq::bounded(rate, 8000, 384000, 48000);
        self.fade = std::max(1, int(self.rate * 0.005)); reset(p);
    }
    static void process(int count, float* input, float* output, PluginDef* p) {
        auto& self = *static_cast<ArtemisEq*>(p);
        for (unsigned b = 0; b < self.bands.size(); ++b)
            self.bands[b].prepare(artemis_eq::coefficients(self.settings[b], self.rate), self.initial, self.fade);
        self.initial = false;
        std::array<artemis_eq::Band*, artemis_eq::band_count> active{};
        unsigned active_count = 0;
        for (auto& band : self.bands)
            if (!band.current.c.flat || band.remaining) active[active_count++] = &band;
        if (!active_count) {
            if (output != input) std::copy(input, input + count, output);
            return;
        }
        // Fixed storage and bounded work; no allocation, locks, FFT or I/O.
        for (int i = 0; i < count; ++i) {
            double sample = input[i];
            for (unsigned b = 0; b < active_count; ++b) sample = active[b]->tick(sample, self.fade);
            output[i] = float(sample);
        }
    }
    static int parameters(const ParamReg& reg) {
        auto& self = *static_cast<ArtemisEq*>(reg.plugin);
        static const value_pair shapes[] = {{"Bell", "Bell"}, {"Low shelf", "Low shelf"}, {"High shelf", "High shelf"}, {"Low cut", "Low cut"}, {"High cut", "High cut"}, {nullptr, nullptr}};
        for (unsigned i = 0; i < self.settings.size(); ++i) {
            auto& s = self.settings[i];
            const std::string prefix = "artemis_eq.b" + std::to_string(i + 1) + "_";
            const auto add = [&](const char* key, float* value, float initial, float low, float high, float step, const value_pair* labels = nullptr) {
                reg.registerFloatVar((prefix + key).c_str(), key, key[0] == 'e' ? "B" : "S", "Artemis EQ band", value, initial, low, high, step, labels);
            };
            add("enabled", &s.enabled, 0, 0, 1, 1);
            add("frequency", &s.frequency, 1000, 20, 20000, 1);
            add("gain", &s.gain, 0, -24, 24, 0.1f);
            add("q", &s.q, 1, 0.1f, 18, 0.01f);
            add("shape", &s.shape, 0, 0, 4, 1, shapes);
        }
        return 0;
    }
    static int ui(const UiBuilder& b, int form) {
        if (!(form & UI_FORM_STACK)) return -1;
        for (unsigned i = 1; i <= artemis_eq::band_count; ++i) {
            const std::string prefix = "artemis_eq.b" + std::to_string(i) + "_";
            b.openHorizontalBox(("Band " + std::to_string(i)).c_str());
            b.create_switch("switch", (prefix + "enabled").c_str(), "On");
            b.create_small_rackknobr((prefix + "frequency").c_str(), "Hz");
            b.create_small_rackknobr((prefix + "gain").c_str(), "dB");
            b.create_small_rackknobr((prefix + "q").c_str(), "Q");
            b.create_selector((prefix + "shape").c_str(), "Shape"); b.closeBox();
        }
        return 0;
    }
    static void destroy(PluginDef* p) { delete static_cast<ArtemisEq*>(p); }
public:
    ArtemisEq() : PluginDef() {
        version = PLUGINDEF_VERSION; id = "artemis_eq"; name = "Artemis EQ";
        category = "EQ"; description = "16 freely placed bell, shelf and high/low cut bands";
        mono_audio = process; set_samplerate = init; clear_state = reset;
        activate_plugin = activate; register_params = parameters; load_ui = ui; delete_instance = destroy;
    }
};
} // namespace gx_engine
