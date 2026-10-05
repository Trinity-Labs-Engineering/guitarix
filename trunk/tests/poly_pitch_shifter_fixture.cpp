#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <new>
#include <vector>

#include "gx_poly_pitch_shifter.h"

namespace {

bool count_allocations = false;
size_t callback_allocations = 0;
const double pi2 = 6.28318530717958647692;

double tone_amplitude(const std::vector<float>& signal, int start,
                      double frequency, int sample_rate)
{
    double real = 0.0;
    double imaginary = 0.0;
    const int length = static_cast<int>(signal.size()) - start;
    for (int i = start; i < static_cast<int>(signal.size()); ++i) {
        const double phase = pi2*frequency*(i - start)/sample_rate;
        real += signal[i]*std::cos(phase);
        imaginary -= signal[i]*std::sin(phase);
    }
    return 2.0*std::sqrt(real*real + imaginary*imaginary)/length;
}

double strongest_near(const std::vector<float>& signal, int start,
                      double frequency, int sample_rate)
{
    double strongest = 0.0;
    for (double probe = frequency - 5.0; probe <= frequency + 5.0; probe += 0.5)
        strongest = std::max(strongest,
            tone_amplitude(signal, start, probe, sample_rate));
    return strongest;
}

std::vector<float> sine(int length, int sample_rate, double frequency)
{
    std::vector<float> signal(length);
    for (int i = 0; i < length; ++i)
        signal[i] = static_cast<float>(0.5*std::sin(pi2*frequency*i/sample_rate));
    return signal;
}

std::vector<float> noise(int length)
{
    std::vector<float> signal(length);
    uint32_t state = 123456789;
    for (int i = 0; i < length; ++i) {
        state = state*1664525u + 1013904223u;
        signal[i] = static_cast<float>((state >> 8)/16777216.0 - 0.5);
    }
    return signal;
}

void process(gx_engine::PolyphonicPitchShifter& shifter,
             const float* input, float* output, int count, double ratio,
             float wet = 1.0f, float dry = 0.0f, bool compensate_dry = false)
{
    // Neither a new search nor an automated ratio may allocate in JACK.
    count_allocations = true;
    shifter.process(input, output, count, ratio, wet, dry,
        compensate_dry, 1.0f, 1.0f, 1.0f, 1.0f);
    count_allocations = false;
    assert(callback_allocations == 0);
}

void render(gx_engine::PolyphonicPitchShifter& shifter,
            const std::vector<float>& input, std::vector<float>& output,
            double ratio, int block_size = 127, bool in_place = false,
            float wet = 1.0f, float dry = 0.0f, bool compensate_dry = false)
{
    if (in_place) output = input;
    for (int position = 0; position < static_cast<int>(input.size());
         position += block_size) {
        const int count = std::min(block_size,
            static_cast<int>(input.size()) - position);
        const float* source = in_place ? &output[position] : &input[position];
        process(shifter, source, &output[position], count, ratio,
            wet, dry, compensate_dry);
    }
}

void assert_equal(const std::vector<float>& actual,
                  const std::vector<float>& expected)
{
    assert(actual.size() == expected.size());
    for (size_t i = 0; i < actual.size(); ++i) {
        assert(std::isfinite(actual[i]));
        assert(std::abs(actual[i] - expected[i]) < 2.0e-6f);
    }
}

void test_pitch_and_chords()
{
    const int sample_rate = 48000;
    const int length = sample_rate*3;
    const std::vector<float> tone = sine(length, sample_rate, 440.0);
    const double ratios[] = {0.25, 0.5, std::pow(2.0, -1.5/12.0), 2.0, 4.0};
    for (double ratio : ratios) {
        gx_engine::PolyphonicPitchShifter shifter;
        shifter.prepare(sample_rate, 1);
        std::vector<float> output(length);
        render(shifter, tone, output, ratio);
        const double shifted = strongest_near(output, sample_rate,
            440.0*ratio, sample_rate);
        assert(shifted > 0.35);
        assert(tone_amplitude(output, sample_rate, 440.0*ratio, sample_rate)
            > 10.0*tone_amplitude(output, sample_rate, 440.0, sample_rate));
    }

    const double chord[] = {110.0, 196.0, 329.627557};
    std::vector<float> input(length, 0.0f);
    for (int i = 0; i < length; ++i)
        for (int note = 0; note < 3; ++note)
            input[i] += static_cast<float>(0.2*std::sin(
                pi2*chord[note]*i/sample_rate + note*0.31));

    for (double ratio : {std::pow(2.0, -2.0/12.0), std::pow(2.0, 2.0/12.0)}) {
        gx_engine::PolyphonicPitchShifter shifter;
        shifter.prepare(sample_rate, 1);
        std::vector<float> output(length);
        render(shifter, input, output, ratio);
        // Preserve every note through common transpose intervals. Upstream
        // correlation splices compromise the individual note pitches of
        // dissonant chords at octave shifts; single notes cover that range.
        for (int note = 0; note < 3; ++note)
            assert(strongest_near(output, sample_rate,
                chord[note]*ratio, sample_rate) > 0.10);
        for (float sample : output) {
            assert(std::isfinite(sample));
            assert(std::abs(sample) < 2.0f);
        }
    }
}

void test_unison_and_rates()
{
    gx_engine::PolyphonicPitchShifter shifter;
    for (int rate : {44100, 48000, 96000}) {
        const std::vector<float> input = noise(rate/4);
        std::vector<float> output(input.size());
        for (int mode = 0; mode < 3; ++mode) {
            shifter.prepare(rate, mode); // Also exercise device-rate changes.
            const int window_ms[] = {60, 30, 20};
            const int floor = static_cast<int>(rate*0.002);
            const int window = static_cast<int>(rate*window_ms[mode]*0.001);
            assert(shifter.latency_samples() == (floor + window)/2);
            render(shifter, input, output, 1.0);
            // TONE3000 starts its tap at the 2 ms floor. Its reported
            // latency is the mean shifting delay, not this unison delay.
            // Ignore the 25 ms fade-in of the wet signal.
            for (int i = rate/10; i < static_cast<int>(input.size()); ++i)
                assert(std::abs(output[i] - input[i - floor]) < 2.0e-6f);
        }
        shifter.prepare(rate, 1);
        const auto tone = sine(rate, rate, 220.0);
        output.resize(tone.size());
        const double ratio = std::pow(2.0, -2.0/12.0);
        render(shifter, tone, output, ratio, 4097);
        // Rate-scaled correlation and fades retain a shifted steady tone's
        // gain (within 1 dB) at the device's actual sample rate.
        const double gain = strongest_near(output, rate/2, 220.0*ratio, rate)/0.5;
        assert(gain > 0.89 && gain < 1.13);
    }
}

void test_blocks_in_place_and_reset()
{
    const auto input = noise(48000);
    const double ratio = std::pow(2.0, -5.0/12.0);
    gx_engine::PolyphonicPitchShifter reference;
    reference.prepare(48000, 1);
    std::vector<float> expected(input.size());
    render(reference, input, expected, ratio);

    gx_engine::PolyphonicPitchShifter shifter;
    for (int block : {1, 16, 512, 4097}) {
        shifter.prepare(48000, 1);
        std::vector<float> output(input.size());
        render(shifter, input, output, ratio, block);
        assert_equal(output, expected);
    }
    shifter.prepare(48000, 1);
    std::vector<float> in_place;
    render(shifter, input, in_place, ratio, 127, true);
    assert_equal(in_place, expected);

    // A reset clears audio, onset history, tap/search/fade state and the
    // previous ratio, including resets partway through a correlation splice.
    process(shifter, input.data(), in_place.data(), 733, 4.0);
    count_allocations = true;
    shifter.reset();
    count_allocations = false;
    assert(callback_allocations == 0);
    render(shifter, input, in_place, ratio);
    assert_equal(in_place, expected);
    shifter.reset();
    std::vector<float> silence(4800, 0.0f);
    std::vector<float> silent_output(silence.size(), 1.0f);
    render(shifter, silence, silent_output, 0.25);
    assert_equal(silent_output, silence);
}

void test_wet_dry()
{
    const auto input = noise(8192);
    std::vector<float> wet(input.size()), output(input.size());
    gx_engine::PolyphonicPitchShifter shifter;
    shifter.prepare(48000, 1);
    for (double ratio : {0.5, 1.0, std::pow(2.0, 7.0/12.0)}) {
        shifter.reset();
        render(shifter, input, wet, ratio);
        for (bool compensate : {false, true}) {
            const int delay = compensate ? shifter.latency_samples() : 0;
            for (bool in_place : {false, true}) {
                for (float wet_gain : {0.0f, 0.25f, 0.5f, 1.0f}) {
                    for (float dry_gain : {0.0f, 0.25f, 0.5f, 1.0f}) {
                        shifter.reset();
                        render(shifter, input, output, ratio, 127, in_place,
                            wet_gain, dry_gain, compensate);
                        for (int i = 0; i < static_cast<int>(input.size()); ++i) {
                            // The dry reference comes directly from the input;
                            // it must never depend on the shifted signal or Wet.
                            const float raw = i >= delay ? input[i - delay] : 0.0f;
                            const float expected = wet_gain*wet[i] + dry_gain*raw;
                            assert(std::abs(output[i] - expected) < 2.0e-6f);
                            if (wet_gain == 0.0f)
                                assert(output[i] == dry_gain*raw);
                        }
                    }
                }
            }
        }
    }
}

void test_wet_only_startup()
{
    // A wet-only unison signal must contain only delayed impulses, even
    // during startup. An upstream dry-to-wet blend leaks each impulse at
    // its original, undelayed position for the first 25 ms.
    std::vector<float> input(4096, 0.0f), output(input.size());
    for (int position : {0, 240, 1000}) input[position] = 0.5f;
    for (bool in_place : {false, true}) {
        gx_engine::PolyphonicPitchShifter shifter;
        shifter.prepare(48000, 1);
        render(shifter, input, output, 1.0, 127, in_place);
        for (int position : {0, 240, 1000})
            assert(output[position] == 0.0f);
        for (int position : {240, 1000})
            assert(output[position + 96] > 0.0f);
    }
}

void test_live_mix_and_mute()
{
    const int block = 512;
    const auto input = noise(block*32);
    const double ratio = std::pow(2.0, -5.0/12.0);
    std::vector<float> wet(input.size()), output(input.size());
    gx_engine::PolyphonicPitchShifter reference;
    reference.prepare(48000, 1);
    render(reference, input, wet, ratio, block);

    for (bool in_place : {false, true}) {
        gx_engine::PolyphonicPitchShifter shifter;
        shifter.prepare(48000, 1);
        if (in_place) output = input;
        const float levels[] = {0.0f, 0.25f, 0.5f, 1.0f};
        // Sweep every combination twice without resetting. Both-zero must
        // mute from startup and immediately after a full-level block; the
        // wet history must remain valid when the user raises a knob again.
        for (int b = 0; b < 32; ++b) {
            const float wet_gain = levels[(b/4)%4];
            const float dry_gain = levels[b%4];
            const int position = b*block;
            const float* source = in_place ? &output[position] : &input[position];
            process(shifter, source, &output[position], block, ratio,
                wet_gain, dry_gain);
            for (int i = position; i < position + block; ++i) {
                const float expected = wet_gain*wet[i] + dry_gain*input[i];
                assert(std::abs(output[i] - expected) < 2.0e-6f);
                if (wet_gain == 0.0f && dry_gain == 0.0f)
                    assert(output[i] == 0.0f);
            }
        }

        count_allocations = true;
        shifter.reset();
        count_allocations = false;
        assert(callback_allocations == 0);
        render(shifter, input, output, ratio, block, in_place, 0.0f, 0.0f);
        for (float sample : output) assert(sample == 0.0f);

        // Clear the accumulated audio before processing genuine silence.
        shifter.reset();
        const std::vector<float> silence(input.size(), 0.0f);
        render(shifter, silence, output, ratio, block, in_place, 1.0f, 1.0f);
        for (float sample : output) assert(sample == 0.0f);
    }
}

void test_unprepared_mix()
{
    const auto input = noise(512);
    std::vector<float> output(input.size());
    gx_engine::PolyphonicPitchShifter shifter;
    for (bool in_place : {false, true}) {
        for (bool compensate : {false, true}) {
            for (float wet_gain : {0.0f, 0.25f, 0.5f, 1.0f}) {
                for (float dry_gain : {0.0f, 0.25f, 0.5f, 1.0f}) {
                    render(shifter, input, output, 2.0, 127, in_place,
                        wet_gain, dry_gain, compensate);
                    for (size_t i = 0; i < input.size(); ++i)
                        assert(output[i] == dry_gain*input[i]);
                }
            }
        }
    }
}

void test_pitch_automation()
{
    const int sample_rate = 48000;
    const int block = 512;
    const int blocks = 4*sample_rate/block;
    const auto input = sine(blocks*block, sample_rate, 220.0);
    std::vector<float> output(input.size());
    gx_engine::PolyphonicPitchShifter shifter;
    shifter.prepare(sample_rate, 1);
    for (int b = 0; b < blocks; ++b) {
        // Same smooth 0 -> +24 -> -24 -> 0 semitone sweep as upstream:
        // searches must survive small changes and restart on direction flips.
        const double phase = static_cast<double>(b)/blocks;
        const double semitones = phase < 0.25 ? 96.0*phase
            : phase < 0.75 ? 24.0 - 96.0*(phase - 0.25)
            : -24.0 + 96.0*(phase - 0.75);
        process(shifter, &input[b*block], &output[b*block], block,
            std::pow(2.0, semitones/12.0));
    }
    for (int i = sample_rate; i < static_cast<int>(output.size()); ++i) {
        assert(std::isfinite(output[i]));
        assert(std::abs(output[i] - output[i - 1]) < 0.12f);
    }
    for (int end = sample_rate; end <= static_cast<int>(output.size()); end += 480) {
        double energy = 0.0;
        for (int i = end - 480; i < end; ++i) energy += output[i]*output[i];
        assert(std::sqrt(energy/480) > 0.15);
    }
}

void test_attack_resync()
{
    const int rate = 48000;
    const double ratio = std::pow(2.0, -2.0/12.0);
    for (int hold_ms : {700, 1600}) {
        const int hold = hold_ms*48;
        auto input = sine(hold + rate/2, rate, 110.0);
        for (float& sample : input) sample *= 0.2f;
        const auto burst = noise(rate/2);
        for (size_t i = 0; i < burst.size(); ++i)
            input[hold + i] = 1.8f*burst[i];
        gx_engine::PolyphonicPitchShifter shifter;
        shifter.prepare(rate, 1);
        std::vector<float> output(input.size());
        render(shifter, input, output, ratio);

        // A pick-like broadband burst after a quiet low sustain should
        // arrive at the floor plus onset-search/fade allowance, regardless
        // of where the drifting read tap sat before the attack.
        int arrival = -1;
        for (int end = hold + 48; end < static_cast<int>(output.size()); ++end) {
            double energy = 0.0;
            for (int i = end - 48; i < end; ++i) energy += output[i]*output[i];
            if (std::sqrt(energy/48) > 0.3) {
                arrival = end;
                break;
            }
        }
        assert(arrival >= hold + 96); // 2 ms floor.
        assert(arrival <= hold + 96 + 192 + 96 + 96);
    }
}

} // namespace

void *operator new(size_t size)
{
    if (count_allocations) ++callback_allocations;
    void *memory = std::malloc(size);
    if (!memory) throw std::bad_alloc();
    return memory;
}

void operator delete(void *memory) noexcept { std::free(memory); }

void *operator new[](size_t size)
{
    if (count_allocations) ++callback_allocations;
    void *memory = std::malloc(size);
    if (!memory) throw std::bad_alloc();
    return memory;
}

void operator delete[](void *memory) noexcept { std::free(memory); }

int main()
{
    test_pitch_and_chords();
    test_unison_and_rates();
    test_blocks_in_place_and_reset();
    test_wet_dry();
    test_wet_only_startup();
    test_live_mix_and_mute();
    test_unprepared_mix();
    test_pitch_automation();
    test_attack_resync();
    std::cout << "poly-pitch-shifter-ok\n";
    return 0;
}
