// Copyright (C) 2026 Trinity Labs Engineering
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <tuple>
#include "gx_plugin.h"

namespace audition {
constexpr double pi = 3.14159265358979323846;
inline double db(double x) { return std::pow(10.0, x / 20.0); }
inline double logPot(double x) { return (std::pow(10.0, x) - 1.0) / 9.0; }
using Params = std::array<double, 8>;

template<int N> struct Filter {
    std::array<double, N + 1> b{}, a{};
    std::array<double, N> z{};
    void reset() { z.fill(0); }
    double tick(double x) {
        const double y = b[0] * x + z[0];
        for (int i = 0; i < N - 1; ++i) z[i] = b[i+1] * x - a[i+1] * y + z[i+1];
        z[N-1] = b[N] * x - a[N] * y;
        return y;
    }
    // Coefficients of descending powers of s, Tustin transform s=K(1-z^-1)/(1+z^-1).
    void analog(const std::array<double, N+1>& bs, const std::array<double, N+1>& as, double K) {
        b.fill(0); a.fill(0);
        for (int term = 0; term <= N; ++term) {
            std::array<double, N+1> poly{}; poly[0] = 1;
            for (int factor = 0; factor < N; ++factor) {
                double sign = factor < N - term ? -1 : 1;
                for (int j = factor+1; j > 0; --j) poly[j] += sign * poly[j-1];
            }
            double scale = std::pow(K, N-term);
            for (int j = 0; j <= N; ++j) { b[j] += bs[term]*scale*poly[j]; a[j] += as[term]*scale*poly[j]; }
        }
        double norm = a[0];
        for (int j = 0; j <= N; ++j) { b[j] /= norm; a[j] /= norm; }
    }
};
struct OnePole : Filter<1> {
    void lowpass(double hz, double fs) {
        double k = std::tan(pi * std::clamp(hz/fs, 0.000001, 0.45));
        b = {k/(1+k), k/(1+k)}; a = {1, (k-1)/(k+1)};
    }
    void highpass(double hz, double fs) {
        double k = std::tan(pi * std::clamp(hz/fs, 0.000001, 0.45));
        b = {1/(1+k), -1/(1+k)}; a = {1, (k-1)/(k+1)};
    }
};

// Causal, fixed-storage sample-rate conversion. No block buffering or allocations.
// Both filters retain 16 samples of group delay in their respective input rates.
class SincHistory {
    static constexpr int phases = 128, taps = 33, size = 256;
    std::array<double, size> samples{};
    std::array<std::array<float,taps>, phases+1> coefficients{};
    int64_t written = 0;
public:
    void prepare(double ratio) {
        const double cutoff = 0.9 * std::min(1.0, ratio);
        for (int p = 0; p <= phases; ++p) {
            double sum = 0;
            for (int k = 0; k < taps; ++k) {
                double x = k + double(p)/phases - 16;
                double w = std::abs(x) > 16 ? 0 : 0.42 + 0.5*std::cos(pi*x/16) + 0.08*std::cos(2*pi*x/16);
                double h = (std::abs(x) < 1e-12 ? cutoff : std::sin(pi*cutoff*x)/(pi*x)) * w;
                coefficients[p][k] = h; sum += h;
            }
            for (auto& h : coefficients[p]) h /= sum;
        }
        reset();
    }
    void reset() { samples.fill(0); written = 0; }
    void push(double x) { samples[(written++) & (size-1)] = x; }
    double at(double position) const {
        const auto n = static_cast<int64_t>(std::floor(position));
        double phase = (position - double(n)) * phases;
        int p = std::min(phases-1, int(phase)); double fraction = phase-p;
        double y = 0;
        for (int k = 0; k < taps; ++k) {
            const auto i = n-k;
            if (i >= 0 && i < written && written-i <= size)
                y += samples[i & (size-1)] * (coefficients[p][k] + fraction*(coefficients[p+1][k]-coefficients[p][k]));
        }
        return y;
    }
};
class RateBridge {
    SincHistory input, output;
    double ratio = 1;
    int64_t hostFrame = 0, coreFrame = 0;
public:
    void prepare(double hostRate, double coreRate) {
        ratio = coreRate / hostRate;
        input.prepare(ratio); output.prepare(1/ratio); reset();
    }
    void reset() { input.reset(); output.reset(); hostFrame = coreFrame = 0; }
    template<class Process> double tick(double x, Process process) {
        if (ratio == 1) return process(x);
        input.push(x);
        while (double(coreFrame) <= double(hostFrame)*ratio + 1e-8) {
            output.push(process(input.at(double(coreFrame)/ratio)));
            ++coreFrame;
        }
        // One host sample ensures the interpolation's newest source sample exists,
        // even at non-integer ratios. All clocks persist across callback boundaries.
        double y = output.at(double(hostFrame-1)*ratio);
        ++hostFrame;
        return y;
    }
};

struct Control {
    const char* id;
    const char* name;
    float initial, low, high, step;
    const char* help = "";
    const value_pair* choices = nullptr;
};

template<class Core, size_t Count> class Pedal : public PluginDef {
    Core core;
    const std::array<Control, Count>& definitions;
    std::array<float, Count> values{};
    Params smoothed{};
    double rate = 48000, coefficient = 0;
    unsigned clock = 0;
    double fade = 0;
    static int params(const ParamReg& reg) {
        auto& s = *static_cast<Pedal*>(reg.plugin);
        for (size_t i = 0; i < Count; ++i) {
            const auto& c = s.definitions[i];
            reg.registerFloatVar(c.id, c.name, "S", c.help, &s.values[i], c.initial, c.low, c.high, c.step, c.choices);
        }
        return 0;
    }
    void seed() {
        for (size_t i = 0; i < Count; ++i) smoothed[i] = std::clamp(values[i], definitions[i].low, definitions[i].high);
        clock = 0; fade = 0;
    }
    static void init(unsigned fs, PluginDef* p) {
        auto& s = *static_cast<Pedal*>(p);
        s.rate = std::clamp(double(fs), 8000.0, 192000.0);
        s.coefficient = 1 - std::exp(-1.0/(0.015*s.rate));
        s.seed(); s.core.prepare(s.rate, s.smoothed);
    }
    static void clear(PluginDef* p) {
        auto& s = *static_cast<Pedal*>(p);
        s.seed(); s.core.configure(s.smoothed); s.core.reset(); s.core.configure(s.smoothed);
    }
    static int activate(bool, PluginDef* p) { clear(p); return 0; }
    static void process(int n, float* in, float* out, PluginDef* p) {
        auto& s = *static_cast<Pedal*>(p);
        for (int j = 0; j < n; ++j) {
            for (size_t i = 0; i < Count; ++i) {
                const auto& c = s.definitions[i];
                double target = std::isfinite(s.values[i]) ? std::clamp(s.values[i], c.low, c.high) : c.initial;
                if (c.step == 1 && c.high-c.low <= 10) s.smoothed[i] = std::round(target);
                else s.smoothed[i] += s.coefficient * (target-s.smoothed[i]);
            }
            if ((s.clock++ & 15) == 0) s.core.configure(s.smoothed);
            double x = std::isfinite(in[j]) ? std::clamp(double(in[j]), -16.0, 16.0) : 0;
            double y = s.core.tick(x);
            s.fade = std::min(1.0, s.fade + 1.0/(0.005*s.rate));
            out[j] = float(y * s.fade);
        }
    }
    static int ui(const UiBuilder& b, int form) {
        if (!(form & UI_FORM_STACK)) return -1;
        auto& s = *static_cast<Pedal*>(b.plugin);
        b.openHorizontalBox("");
        for (const auto& c : s.definitions) {
            if (c.choices) b.create_selector(c.id, c.name);
            else b.create_small_rackknob(c.id, c.name);
        }
        b.closeBox(); return 0;
    }
    static void destroy(PluginDef* p) { delete static_cast<Pedal*>(p); }
public:
    Pedal(const char* key, const char* label, const char* detail, const std::array<Control, Count>& controls)
        : PluginDef(), definitions(controls) {
        version = PLUGINDEF_VERSION; id = key; name = label; category = "Audition"; description = detail;
        mono_audio = process; set_samplerate = init; clear_state = clear;
        activate_plugin = activate; register_params = params; load_ui = ui; delete_instance = destroy;
        for (size_t i = 0; i < Count; ++i) values[i] = controls[i].initial;
    }
};
} // namespace audition
