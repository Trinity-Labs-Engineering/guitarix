/* Copyright (C) 2026 Trinity Labs Engineering
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>

namespace gx_engine {
namespace harmony {

// DOCS/MODEPY/MODE_ENCODING.py is the reference. Notes are A=0 .. G#=11;
// modes retain dictionary order; the interval pot is -7 .. +7 diatonic steps.
inline int interval_shift(int note, int tonic, int mode, int interval) {
    static const char* const patterns[] = {
        "110111", "101110", "011101", "111011", "110110", "101101", "011011"
    };
    interval = std::max(-7, std::min(7, interval));
    if (!interval) return 0;
    const int direction = interval > 0 ? 1 : -1;
    if (std::abs(interval) == 7) return direction * 12;
    tonic = (tonic % 12 + 12) % 12;
    mode = std::max(0, std::min(6, mode));
    int notes[7] = {tonic}, index = -1;
    for (int i = 1; i < 7; ++i)
        notes[i] = (notes[i - 1] + (patterns[mode][i - 1] == '1' ? 2 : 1)) % 12;
    for (int i = 0; i < 7; ++i) if (notes[i] == note) index = i;
    if (index < 0) return direction * 12; // intentional out-of-mode behaviour
    const int target = notes[(index + interval + 7) % 7];
    return direction > 0 ? (target - note + 12) % 12 : -((note - target + 12) % 12);
}

// Streaming YIN: update the difference sums as the integration window slides,
// rather than doing a quadratic analysis burst on the audio callback. Four
// low-pass views favour the lowest audible fundamental of a chord. Each view
// uses the FIRST YIN valley, avoiding the suboctaves of a periodic note.
// A windowed spectral check identifies the lowest actual component of chords;
// YIN alone would follow their common subharmonic or an upper voice. The short
// YIN path acquires single notes first; the spectral path needs ~128 ms history.
// No allocation, locking, external FFT dependency or worker-thread scheduling.
class FundamentalTracker {
    static constexpr int ring_size = 1024;
    static constexpr int spectral_size = 768;
    static constexpr int window = 160;
    static constexpr int max_lag = 160;
    struct Band {
        std::array<float, ring_size> ring{};
        std::array<double, max_lag + 1> difference{};
        std::array<double, 4> filter{};
        double coefficient = 0, energy = 0;
    };
    std::array<Band, 4> bands{};
    int decimation = 8, decimation_phase = 0, position = 0, filled = 0;
    int lag_max = 110, lag_min = 4, hop = 32, until_analysis = 32;
    int midi = -1, candidate = -1, candidate_count = 0, missed = 0;
    double analysis_rate = 6000, last_frequency = 0;
    std::array<float, ring_size> spectral_ring{};
    std::array<double, spectral_size> spectral_window{};
    std::array<std::complex<double>, 2048> spectrum{};
    std::array<std::complex<double>, 1024> twiddles{};
    int spectral_filled = 0, spectral_hop = 0;
    double spectral_frequency = 0;

    void analyse_spectrum() {
        // Zero padding interpolates peaks; a Kaiser window resolves close low chord tones while rejecting sidelobes.
        double mean = 0;
        for (int i = 0; i < spectral_size; ++i)
            mean += spectral_ring[(position - spectral_size + i) & (ring_size - 1)];
        mean /= spectral_size;
        for (int i = 0; i < 2048; ++i)
            spectrum[i] = i < spectral_size ? (spectral_ring[(position - spectral_size + i) & (ring_size - 1)] - mean) * spectral_window[i] : 0;
        for (int i = 1, j = 0; i < 2048; ++i) {
            int bit = 1024;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(spectrum[i], spectrum[j]);
        }
        for (int length = 2; length <= 2048; length <<= 1) {
            for (int start = 0; start < 2048; start += length) {
                for (int j = 0; j < length / 2; ++j) {
                    const auto a = spectrum[start + j];
                    const auto b = spectrum[start + j + length / 2] * twiddles[j * (2048 / length)];
                    spectrum[start + j] = a + b;
                    spectrum[start + j + length / 2] = a - b;
                }
            }
        }
        const double bin_hz = analysis_rate / 2048;
        const int begin = std::max(2, static_cast<int>(55 / bin_hz));
        const int end = std::min(1022, static_cast<int>(1500 / bin_hz) + 1);
        double maximum = 0;
        for (int i = begin; i <= end; ++i) maximum = std::max(maximum, std::norm(spectrum[i]));
        spectral_frequency = 0;
        if (maximum < 1e-5) return;
        for (int i = begin; i < end; ++i) {
            const double power = std::norm(spectrum[i]);
            if (power < maximum * 0.01 || power <= std::norm(spectrum[i - 1]) || power < std::norm(spectrum[i + 1])) continue;
            const double left = std::log(std::max(1e-20, std::norm(spectrum[i - 1])));
            const double centre = std::log(power);
            const double right = std::log(std::max(1e-20, std::norm(spectrum[i + 1])));
            const double delta = 0.5 * (left - right) / (left - 2 * centre + right);
            const double frequency = (i + delta) * bin_hz;
            if (frequency >= 55 && frequency <= 1500) { spectral_frequency = frequency; break; }
        }
    }

    void analyse() {
        if (spectral_filled == spectral_size && ++spectral_hop >= 4) {
            spectral_hop = 0;
            analyse_spectrum();
        }
        double lowest = 0;
        for (const auto& band : bands) {
            if (band.energy / window < 1e-10) continue;
            std::array<double, max_lag + 1> yin{};
            double sum = 0;
            yin[0] = 1;
            for (int tau = 1; tau <= lag_max; ++tau) {
                const double diff = std::max(0.0, band.difference[tau]);
                sum += diff;
                yin[tau] = sum > 1e-16 ? diff * tau / sum : 1;
            }
            // A DC input has energy but no pitch. Ignore cancellation residue
            // in its rolling difference sums instead of treating it as a valley.
            if (sum < band.energy * 1e-5) continue;
            int best = -1;
            for (int tau = lag_min; tau < lag_max; ++tau) {
                if (yin[tau] < 0.15) {
                    while (tau + 1 <= lag_max && yin[tau + 1] < yin[tau]) ++tau;
                    best = tau;
                    break;
                }
            }
            if (best < 0 || best >= lag_max) continue;
            const double denominator = yin[best - 1] - 2 * yin[best] + yin[best + 1];
            const double correction = std::abs(denominator) > 1e-12
                ? std::max(-0.5, std::min(0.5, 0.5 * (yin[best - 1] - yin[best + 1]) / denominator)) : 0;
            const double frequency = analysis_rate / (best + correction);
            // The spectrum rejects virtual subharmonics and upper chord tones.
            const bool matches = !spectral_frequency || std::abs(12 * std::log2(frequency / spectral_frequency)) < 0.45;
            if (matches && frequency >= 55 && frequency <= 1500 && (!lowest || frequency < lowest)) lowest = frequency;
        }
        if (!lowest) lowest = spectral_frequency;
        if (!lowest) {
            candidate = -1; candidate_count = 0;
            // Hold through brief attacks/dropouts, then mute the untracked wet path.
            if (++missed * hop > analysis_rate * 0.1) { midi = -1; last_frequency = 0; }
            return;
        }
        missed = 0;
        const double detected = 69 + 12 * std::log2(lowest / 440.0);
        if (midi >= 0 && std::abs(detected - midi) < 0.65) {
            candidate = midi; candidate_count = 0; last_frequency = lowest;
            return;
        }
        const int rounded = static_cast<int>(std::lround(detected));
        if (candidate != rounded) { candidate = rounded; candidate_count = 0; }
        if (++candidate_count >= 2) { midi = rounded; last_frequency = lowest; }
    }

public:
    void prepare(double rate) {
        rate = rate > 0 ? rate : 48000;
        decimation = std::max(1, static_cast<int>(std::lround(rate / 6000)));
        analysis_rate = rate / decimation;
        lag_max = std::min(int(max_lag), static_cast<int>(std::ceil(analysis_rate / 55)) + 1);
        lag_min = std::max(2, static_cast<int>(analysis_rate / 1500));
        hop = std::max(1, static_cast<int>(std::lround(analysis_rate * 0.005)));
        const double cutoffs[] = {120, 240, 480, 1600};
        for (int i = 0; i < 4; ++i)
            bands[i].coefficient = 1 - std::exp(-2 * 3.14159265358979323846 * cutoffs[i] / rate);
        const auto bessel = [](double x) {
            double sum = 1, term = 1;
            for (int k = 1; k < 16; ++k) { term *= x * x / (4 * k * k); sum += term; }
            return sum;
        };
        for (int i = 0; i < spectral_size; ++i) {
            const double x = 2.0 * i / (spectral_size - 1) - 1;
            spectral_window[i] = bessel(3 * std::sqrt(std::max(0.0, 1 - x * x))) / bessel(3);
        }
        for (int i = 0; i < 1024; ++i) twiddles[i] = std::polar(1.0, -2 * 3.14159265358979323846 * i / 2048);
        reset();
    }
    void reset() {
        for (auto& band : bands) {
            band.ring.fill(0); band.difference.fill(0); band.filter.fill(0); band.energy = 0;
        }
        decimation_phase = position = filled = candidate_count = missed = 0;
        midi = candidate = -1; last_frequency = 0; until_analysis = hop;
        spectral_ring.fill(0); spectral_filled = spectral_hop = 0; spectral_frequency = 0;
    }
    int note() const { return midi < 0 ? -1 : (midi + 3) % 12; }
    double frequency() const { return last_frequency; }
    int samples_until_analysis() const { return until_analysis * decimation - decimation_phase; }
    void push(const float* input, int count) {
        for (int i = 0; i < count; ++i) {
            const bool sample = ++decimation_phase == decimation;
            for (auto& band : bands) {
                double value = std::isfinite(input[i]) ? input[i] : 0;
                for (auto& pole : band.filter) {
                    pole += band.coefficient * (value - pole); value = pole;
                }
                if (!sample) continue;
                const float current = static_cast<float>(value);
                const float old = band.ring[(position - window) & (ring_size - 1)];
                band.energy += double(current) * current - double(old) * old;
                for (int tau = 1; tau <= lag_max; ++tau) {
                    const double add = current - band.ring[(position - tau) & (ring_size - 1)];
                    const double remove = old - band.ring[(position - window - tau) & (ring_size - 1)];
                    band.difference[tau] += add * add - remove * remove;
                }
                band.ring[position] = current;
            }
            if (!sample) continue;
            decimation_phase = 0;
            spectral_ring[position] = bands.back().ring[position];
            spectral_filled = std::min(int(spectral_size), spectral_filled + 1);
            position = (position + 1) & (ring_size - 1);
            filled = std::min(window + lag_max, filled + 1);
            if (--until_analysis == 0) {
                until_analysis = hop;
                if (filled == window + lag_max) analyse();
            }
        }
    }
};

} // namespace harmony
} // namespace gx_engine
