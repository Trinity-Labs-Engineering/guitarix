#pragma once

#include <complex>
#include <optional>
#include <vector>

// YIN's fixed-width squared difference and cumulative mean normalisation.
// FFTs only accelerate the correlation term: this is not a spectral peak picker.
// Owned and called exclusively by the analysis thread.
class YinTracker
{
public:
    struct Result
    {
        float tau { 0.f };
        int integerTau { 0 };
        float frequencyHz { 0.f };
        float cmndf { 1.f };
        float clarity { 0.f };
    };

    float threshold { 0.10f };
    std::optional<Result> run (const float* samples, int count,
                               int sampleRate, int minTau, int maxTau);

private:
    using Complex = std::complex<double>;
    std::vector<Complex> signal, window, roots;
    std::vector<int> reversed;
    std::vector<double> energy, difference, cmndf;
    void prepareFFT (int size);
    void fft (std::vector<Complex>& data, bool inverse) const;
    static double interpolate (const std::vector<double>& values, int tau);
    static double fractionalValue (const std::vector<double>& values, double tau);
    static double refine (const std::vector<double>& values, int tau);
    static double minimum (const std::vector<double>& values, int tau);
};
