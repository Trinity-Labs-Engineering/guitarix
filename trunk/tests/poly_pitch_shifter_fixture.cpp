#include <algorithm>
#include <cassert>
#include <cmath>
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

void render(gx_engine::PolyphonicPitchShifter& shifter,
            const std::vector<float>& input, std::vector<float>& output,
            double ratio, bool in_place)
{
    const int block_size = 127; // Deliberately unrelated to any FFT size.
    if (in_place) output = input;
    for (int position = 0; position < static_cast<int>(input.size());
         position += block_size) {
        const int count = std::min(block_size,
            static_cast<int>(input.size()) - position);
        const float *source = in_place ? &output[position] : &input[position];
        shifter.process(source, &output[position], count, ratio,
            1.0f, 0.0f, true, 1.0f, 1.0f, 1.0f, 1.0f);
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
    const int sample_rate = 48000;
    const int length = sample_rate*2;
    const int analysis_start = sample_rate;
    const double chord[] = {110.0, 196.0, 329.627557};
    std::vector<float> input(length, 0.0f);
    for (int i = 0; i < length; ++i) {
        for (int note = 0; note < 3; ++note) {
            input[i] += static_cast<float>(0.2*std::sin(
                pi2*chord[note]*i/sample_rate + note*0.31));
        }
    }

    for (int direction = 0; direction < 2; ++direction) {
        const double ratio = direction ? 2.0 : 0.5;
        gx_engine::PolyphonicPitchShifter shifter;
        shifter.prepare(sample_rate, 1); // Mode selected by Houston.
        assert(shifter.latency_samples() <= sample_rate*0.026);

        std::vector<float> output(length, 0.0f);
        count_allocations = true;
        callback_allocations = 0;
        render(shifter, input, output, ratio, false);
        count_allocations = false;
        assert(callback_allocations == 0);

        // Every note in a widely-spaced guitar chord must retain a strong
        // component around its independently shifted destination frequency.
        for (int note = 0; note < 3; ++note) {
            assert(strongest_near(output, analysis_start,
                chord[note]*ratio, sample_rate) > 0.10);
        }
        for (size_t i = analysis_start; i < output.size(); ++i)
            assert(std::isfinite(output[i]));

        // Guitarix commonly processes mono effects in-place.
        gx_engine::PolyphonicPitchShifter in_place_shifter;
        in_place_shifter.prepare(sample_rate, 1);
        std::vector<float> in_place_output;
        render(in_place_shifter, input, in_place_output, ratio, true);
        assert(in_place_output.size() == output.size());
        for (size_t i = 0; i < output.size(); ++i)
            assert(std::abs(in_place_output[i] - output[i]) < 1.0e-6f);
    }

    // At unison the wet path is an exact, compensated delay rather than a
    // collection of stationary read heads which would comb-filter the input.
    gx_engine::PolyphonicPitchShifter unison;
    unison.prepare(sample_rate, 1);
    std::vector<float> unison_output(length, 0.0f);
    render(unison, input, unison_output, 1.0, false);
    const int delay = unison.latency_samples();
    for (int i = delay; i < length; ++i)
        assert(std::abs(unison_output[i] - input[i - delay]) < 2.0e-6f);

    // Returning an automated shift to zero must also converge to that exact
    // delay, rather than leaving the read head at an arbitrary sweep phase.
    gx_engine::PolyphonicPitchShifter automated;
    automated.prepare(sample_rate, 1);
    std::vector<float> automated_output(length, 0.0f);
    automated.process(&input[0], &automated_output[0], length/2, 0.5,
        1.0f, 0.0f, true, 1.0f, 1.0f, 1.0f, 1.0f);
    automated.process(&input[length/2], &automated_output[length/2], length/2,
        1.0, 1.0f, 0.0f, true, 1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = length - sample_rate/4; i < length; ++i)
        assert(std::abs(automated_output[i] - input[i - delay]) < 2.0e-5f);

    std::cout << "poly-pitch-shifter-ok\n";
    return 0;
}
