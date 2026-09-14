/*
 * Copyright (C) 2026 Trinity Labs Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <stdint.h>
#include <vector>

namespace gx_engine {

/*
 * A low-latency, polyphonic SOLA pitch shifter.
 *
 * The primary read head traverses a circular delay at the pitch ratio.  When
 * it reaches either end of a bounded delay range, a second head is placed at
 * the other end and a short raised-cosine crossfade moves to it.  Before that
 * crossfade, a normalised-correlation search aligns the new waveform with the
 * outgoing one.  No fundamental frequency is selected, so the complete chord
 * is shifted together rather than being forced to a detected note.
 *
 * prepare() is the only allocating operation.  process() is suitable for the
 * JACK callback, supports arbitrary block sizes, and permits in-place use.
 */
class PolyphonicPitchShifter {
private:
    struct Biquad {
        float b0, b1, b2, a1, a2;
        float z1, z2;

        Biquad()
            : b0(1.0f), b1(0.0f), b2(0.0f), a1(0.0f), a2(0.0f),
              z1(0.0f), z2(0.0f) {}

        void reset() { z1 = z2 = 0.0f; }

        void set_lowpass(double sample_rate, double frequency, double q)
        {
            const double pi = 3.14159265358979323846;
            const double omega = 2.0*pi*frequency/sample_rate;
            const double cosine = std::cos(omega);
            const double alpha = std::sin(omega)/(2.0*q);
            const double scale = 1.0/(1.0 + alpha);
            b0 = static_cast<float>(0.5*(1.0 - cosine)*scale);
            b1 = static_cast<float>((1.0 - cosine)*scale);
            b2 = b0;
            a1 = static_cast<float>(-2.0*cosine*scale);
            a2 = static_cast<float>((1.0 - alpha)*scale);
        }

        float process(float input)
        {
            const float output = b0*input + z1;
            z1 = b1*input - a1*output + z2;
            z2 = b2*input - a2*output;
            return output;
        }
    };

    std::vector<float> input_ring;
    std::vector<float> dry_ring;
    Biquad anti_alias[2];

    int sample_rate;
    int ring_mask;
    int64_t write_position;
    int minimum_delay;
    int maximum_delay;
    int search_samples;
    int correlation_samples;
    int crossfade_samples;
    int crossfade_position;
    int compensated_delay;
    double primary_position;
    double secondary_position;
    double smoothed_ratio;
    double ratio_smoothing;
    float tone_low;
    float tone_low_mid;
    float tone_high_mid;
    float tone_low_coefficient;
    float tone_low_mid_coefficient;
    float tone_high_mid_coefficient;

    static int next_power_of_two(int value)
    {
        int result = 1;
        while (result < value) result <<= 1;
        return result;
    }

    float ring_sample(const std::vector<float>& ring, int64_t position) const
    {
        if (position < 0 || position > write_position) return 0.0f;
        return ring[static_cast<size_t>(position) & ring_mask];
    }

    float interpolated_input(double position) const
    {
        const int64_t centre = static_cast<int64_t>(std::floor(position));
        const float fraction = static_cast<float>(position - centre);
        const float y0 = ring_sample(input_ring, centre - 1);
        const float y1 = ring_sample(input_ring, centre);
        const float y2 = ring_sample(input_ring, centre + 1);
        const float y3 = ring_sample(input_ring, centre + 2);

        // Four-point Catmull-Rom interpolation avoids the octave-up roughness
        // and high-frequency loss of a linearly interpolated delay tap.
        const float c0 = y1;
        const float c1 = 0.5f*(y2 - y0);
        const float c2 = y0 - 2.5f*y1 + 2.0f*y2 - 0.5f*y3;
        const float c3 = 0.5f*(y3 - y0) + 1.5f*(y1 - y2);
        return ((c3*fraction + c2)*fraction + c1)*fraction + c0;
    }

    float correlation_score(double outgoing, double candidate,
                            double ratio, int stride) const
    {
        double sum_a = 0.0, sum_b = 0.0;
        double sum_aa = 0.0, sum_bb = 0.0, sum_ab = 0.0;
        int samples = 0;
        for (int n = 0; n < correlation_samples; n += stride) {
            const float a = interpolated_input(outgoing - ratio*n);
            const float b = interpolated_input(candidate - ratio*n);
            sum_a += a;
            sum_b += b;
            sum_aa += a*a;
            sum_bb += b*b;
            sum_ab += a*b;
            ++samples;
        }
        if (samples < 2) return -2.0f;
        const double covariance = sum_ab - sum_a*sum_b/samples;
        const double energy_a = sum_aa - sum_a*sum_a/samples;
        const double energy_b = sum_bb - sum_b*sum_b/samples;
        if (energy_a < 1.0e-8 || energy_b < 1.0e-8) return -2.0f;
        return static_cast<float>(covariance/std::sqrt(energy_a*energy_b));
    }

    double find_alignment(double outgoing, double nominal,
                          double ratio) const
    {
        if (write_position < maximum_delay + correlation_samples)
            return nominal;

        int best_offset = 0;
        float best_score = -2.0f;
        // Coarse/fine searching limits callback cost.  The small distance
        // penalty stops low-energy passages wandering for negligible benefit.
        for (int offset = -search_samples; offset <= search_samples; offset += 4) {
            const float raw = correlation_score(
                outgoing, nominal + offset, ratio, 2);
            const float score = raw - 0.02f*std::abs(offset)/search_samples;
            if (score > best_score) {
                best_score = score;
                best_offset = offset;
            }
        }
        const int fine_start = std::max(-search_samples, best_offset - 4);
        const int fine_end = std::min(search_samples, best_offset + 4);
        for (int offset = fine_start; offset <= fine_end; ++offset) {
            const float raw = correlation_score(
                outgoing, nominal + offset, ratio, 1);
            const float score = raw - 0.02f*std::abs(offset)/search_samples;
            if (score > best_score) {
                best_score = score;
                best_offset = offset;
            }
        }
        return nominal + (best_score > 0.08f ? best_offset : 0);
    }

    float colour(float input, float low, float low_mid,
                 float high_mid, float high)
    {
        const float low_band = tone_low +=
            tone_low_coefficient*(input - tone_low);
        const float low_mid_sum = tone_low_mid +=
            tone_low_mid_coefficient*(input - tone_low_mid);
        const float high_mid_sum = tone_high_mid +=
            tone_high_mid_coefficient*(input - tone_high_mid);
        // This crossover reconstructs input exactly when all four legacy
        // Guitarix band controls are at their default value of one.
        return low*low_band
            + low_mid*(low_mid_sum - low_band)
            + high_mid*(high_mid_sum - low_mid_sum)
            + high*(input - high_mid_sum);
    }

    static float one_pole_coefficient(double sample_rate, double frequency)
    {
        const double pi2 = 6.28318530717958647692;
        return static_cast<float>(1.0 - std::exp(-pi2*frequency/sample_rate));
    }

public:
    PolyphonicPitchShifter()
        : sample_rate(0), ring_mask(0), write_position(0), minimum_delay(0),
          maximum_delay(0), search_samples(0), correlation_samples(0),
          crossfade_samples(0), crossfade_position(0), compensated_delay(0),
          primary_position(0.0), secondary_position(0.0),
          smoothed_ratio(1.0), ratio_smoothing(0.0), tone_low(0.0f),
          tone_low_mid(0.0f), tone_high_mid(0.0f),
          tone_low_coefficient(0.0f), tone_low_mid_coefficient(0.0f),
          tone_high_mid_coefficient(0.0f) {}

    void prepare(int new_sample_rate, int quality)
    {
        sample_rate = std::max(8000, new_sample_rate);
        double range_ms = 40.0;
        double minimum_ms = 5.0;
        double search_ms = 4.0;
        double crossfade_ms = 5.0;
        if (quality <= 0) {
            range_ms = 52.0;
            minimum_ms = 7.0;
            search_ms = 6.0;
            crossfade_ms = 7.0;
        } else if (quality >= 2) {
            range_ms = 26.0;
            minimum_ms = 4.0;
            search_ms = 3.0;
            crossfade_ms = 4.0;
        }
        minimum_delay = std::max(16,
            static_cast<int>(sample_rate*minimum_ms*0.001));
        maximum_delay = minimum_delay + std::max(64,
            static_cast<int>(sample_rate*range_ms*0.001));
        search_samples = std::max(8,
            static_cast<int>(sample_rate*search_ms*0.001));
        correlation_samples = std::max(48,
            static_cast<int>(sample_rate*0.006));
        crossfade_samples = std::max(32,
            static_cast<int>(sample_rate*crossfade_ms*0.001));
        compensated_delay = (minimum_delay + maximum_delay)/2;

        const int needed = maximum_delay + search_samples
            + correlation_samples + 16;
        const int ring_size = next_power_of_two(std::max(2048, needed*2));
        input_ring.assign(ring_size, 0.0f);
        dry_ring.assign(ring_size, 0.0f);
        ring_mask = ring_size - 1;
        ratio_smoothing = 1.0 - std::exp(-1.0/(sample_rate*0.008));
        tone_low_coefficient = one_pole_coefficient(sample_rate, 180.0);
        tone_low_mid_coefficient = one_pole_coefficient(sample_rate, 900.0);
        tone_high_mid_coefficient = one_pole_coefficient(sample_rate, 4000.0);
        reset();
    }

    void reset()
    {
        std::fill(input_ring.begin(), input_ring.end(), 0.0f);
        std::fill(dry_ring.begin(), dry_ring.end(), 0.0f);
        write_position = 0;
        primary_position = -compensated_delay;
        secondary_position = primary_position;
        crossfade_position = crossfade_samples;
        smoothed_ratio = 1.0;
        tone_low = tone_low_mid = tone_high_mid = 0.0f;
        for (int i = 0; i < 2; ++i) anti_alias[i].reset();
    }

    int latency_samples() const { return compensated_delay; }

    void process(const float *input, float *output, int count,
                 double target_ratio, float wet, float dry,
                 bool compensate_dry, float low, float low_mid,
                 float high_mid, float high)
    {
        if (sample_rate <= 0 || input_ring.empty()) {
            if (input != output) std::copy(input, input + count, output);
            return;
        }

        target_ratio = std::max(0.5, std::min(2.0, target_ratio));
        const double anti_alias_cutoff = std::min(
            sample_rate*0.47, sample_rate*0.46/std::max(1.0, target_ratio));
        anti_alias[0].set_lowpass(sample_rate, anti_alias_cutoff, 0.5411961);
        anti_alias[1].set_lowpass(sample_rate, anti_alias_cutoff, 1.3065630);

        const double pi = 3.14159265358979323846;
        for (int sample = 0; sample < count; ++sample, ++write_position) {
            const float raw = input[sample];
            float filtered = anti_alias[0].process(raw);
            filtered = anti_alias[1].process(filtered);
            if (target_ratio <= 1.0001) filtered = raw;
            input_ring[static_cast<size_t>(write_position) & ring_mask] = filtered;
            dry_ring[static_cast<size_t>(write_position) & ring_mask] = raw;

            smoothed_ratio += ratio_smoothing*(target_ratio - smoothed_ratio);
            if (crossfade_position >= crossfade_samples) {
                const double current_delay = write_position - primary_position;
                const bool reset_down = smoothed_ratio < 0.9999
                    && current_delay >= maximum_delay;
                const bool reset_up = smoothed_ratio > 1.0001
                    && current_delay <= minimum_delay;
                if (reset_down || reset_up) {
                    const int target_delay = reset_down
                        ? minimum_delay : maximum_delay;
                    const double nominal = write_position - target_delay;
                    secondary_position = find_alignment(
                        primary_position, nominal, smoothed_ratio);
                    crossfade_position = 0;
                }
            }

            float shifted;
            if (crossfade_position < crossfade_samples) {
                const double progress = static_cast<double>(
                    crossfade_position + 1)/(crossfade_samples + 1);
                const float new_weight = static_cast<float>(
                    0.5 - 0.5*std::cos(pi*progress));
                shifted = (1.0f - new_weight)*interpolated_input(primary_position)
                    + new_weight*interpolated_input(secondary_position);
                primary_position += smoothed_ratio;
                secondary_position += smoothed_ratio;
                if (++crossfade_position >= crossfade_samples)
                    primary_position = secondary_position;
            } else {
                shifted = interpolated_input(primary_position);
                primary_position += smoothed_ratio;
            }

            const float delayed = ring_sample(dry_ring,
                write_position - compensated_delay);
            // When automation returns to zero semitones, converge to the
            // declared compensated delay instead of freezing at whichever
            // point in the delay sweep happened to be active.
            const double semitone_distance = 12.0*std::abs(
                std::log(smoothed_ratio)/std::log(2.0));
            const float shifted_mix = static_cast<float>(
                std::min(1.0, semitone_distance/0.35));
            shifted = shifted_mix*shifted + (1.0f - shifted_mix)*delayed;
            shifted = colour(shifted, low, low_mid, high_mid, high);
            const float dry_sample = compensate_dry ? delayed : raw;
            output[sample] = wet*shifted + dry*dry_sample;
        }
    }
};

} // namespace gx_engine
