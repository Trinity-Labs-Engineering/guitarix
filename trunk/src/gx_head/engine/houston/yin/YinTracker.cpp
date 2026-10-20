#include "YinTracker.h"

#include <algorithm>
#include <cmath>
#include <numeric>

void YinTracker::prepareFFT (int size)
{
    if ((int)signal.size() == size)
        return;
    signal.resize (size);
    window.resize (size);
    reversed.resize (size);
    roots.resize (size / 2);
    for (int i = 0, j = 0; i < size; ++i)
    {
        reversed[i] = j;
        int bit = size >> 1;
        for (; bit && (j & bit); bit >>= 1)
            j ^= bit;
        j ^= bit;
    }
    constexpr double pi = 3.14159265358979323846;
    for (int i = 0; i < size / 2; ++i)
        roots[i] = std::polar (1.0, -2.0 * pi * i / size);
}

void YinTracker::fft (std::vector<Complex>& data, bool inverse) const
{
    const int size = (int)data.size();
    for (int i = 0; i < size; ++i)
        if (i < reversed[i])
            std::swap (data[i], data[reversed[i]]);
    for (int length = 2; length <= size; length *= 2)
    {
        const int half = length / 2, stride = size / length;
        for (int start = 0; start < size; start += length)
            for (int j = 0; j < half; ++j)
            {
                const auto root = inverse ? std::conj (roots[j * stride]) : roots[j * stride];
                const auto even = data[start + j];
                const auto odd = data[start + j + half] * root;
                data[start + j] = even + odd;
                data[start + j + half] = even - odd;
            }
    }
    if (inverse)
        for (auto& value : data)
            value /= size;
}

double YinTracker::interpolate (const std::vector<double>& values, int tau)
{
    const double curvature = values[tau - 1] - 2.0 * values[tau] + values[tau + 1];
    return curvature > 1e-15
        ? tau + std::clamp (0.5 * (values[tau - 1] - values[tau + 1]) / curvature, -0.5, 0.5)
        : tau;
}

double YinTracker::minimum (const std::vector<double>& values, int tau)
{
    const double offset = interpolate (values, tau) - tau;
    return std::max (0.0, values[tau] + 0.25 * (values[tau + 1] - values[tau - 1]) * offset);
}

// Windowed sinc interpolation avoids the parabolic height bias that otherwise
// makes strong upper partials favour a multiple of the true period. Only short
// lags need it; long periods already have sub-cent sample resolution.
double YinTracker::fractionalValue (const std::vector<double>& values, double tau)
{
    constexpr double pi = 3.14159265358979323846;
    constexpr int radius = 12;
    const int centre = (int)std::floor (tau);
    double sum = 0.0, weight = 0.0;
    for (int i = centre - radius + 1; i <= centre + radius; ++i)
    {
        const double distance = tau - i;
        const double x = pi * distance;
        const double sinc = std::abs (x) < 1e-10 ? 1.0 : std::sin (x) / x;
        const double w = sinc * (0.42 + 0.5 * std::cos (x / radius) + 0.08 * std::cos (2.0 * x / radius));
        // At zero lag the difference is zero and locally even. This extension
        // is only needed near the upper frequency search boundary.
        const int index = std::abs (i);
        if (index >= (int)values.size()) continue;
        sum += w * values[index];
        weight += w;
    }
    return sum / weight;
}

double YinTracker::refine (const std::vector<double>& values, int tau)
{
    double low = tau - 0.6, high = tau + 0.6;
    for (int iteration = 0; iteration < 14; ++iteration)
    {
        const double left = low + (high - low) / 3.0;
        const double right = high - (high - low) / 3.0;
        if (fractionalValue (values, left) < fractionalValue (values, right)) high = right;
        else low = left;
    }
    return (low + high) * 0.5;
}

std::optional<YinTracker::Result> YinTracker::run (const float* samples, int count,
                                                  int sampleRate, int minTau, int maxTau)
{
    if (samples == nullptr || count < 16 || sampleRate <= 0)
        return std::nullopt;
    maxTau = std::min (maxTau, count / 2 - 2);
    minTau = std::max (2, minTau);
    if (maxTau <= minTau)
        return std::nullopt;

    // One extra lag allows interpolation at both search boundaries.
    const int width = count - maxTau - 1;
    int fftSize = 1;
    while (fftSize < count)
        fftSize *= 2;
    prepareFFT (fftSize);
    std::fill (signal.begin(), signal.end(), Complex {});
    std::fill (window.begin(), window.end(), Complex {});
    energy.assign (count + 1, 0.0);
    const double mean = std::accumulate (samples, samples + count, 0.0) / count;
    for (int i = 0; i < count; ++i)
    {
        const double value = samples[i] - mean;
        if (! std::isfinite (value))
            return std::nullopt;
        signal[i] = value;
        if (i < width)
            window[i] = value;
        energy[i + 1] = energy[i] + value * value;
    }
    if (energy.back() < 1e-12)
        return std::nullopt;

    fft (signal, false);
    fft (window, false);
    for (int i = 0; i < fftSize; ++i)
        signal[i] *= std::conj (window[i]);
    fft (signal, true);

    // d(tau) = sum(x[j]^2) + sum(x[j+tau]^2) - 2 sum(x[j] x[j+tau]).
    // Zero padding and the rectangular width preserve the exact YIN equation.
    difference.assign (maxTau + 2, 0.0);
    cmndf.assign (maxTau + 2, 1.0);
    double cumulative = 0.0;
    for (int tau = 1; tau <= maxTau + 1; ++tau)
    {
        difference[tau] = std::max (0.0, energy[width] + energy[width + tau]
                                            - energy[tau] - 2.0 * signal[tau].real());
        cumulative += difference[tau];
        if (cumulative > 1e-15)
            cmndf[tau] = difference[tau] * tau / cumulative;
    }

    // Interpolate the HEIGHT before thresholding, as in YIN step 5. Testing
    // only the integer sample misses high notes with a fractional period.
    const auto scoreAt = [&] (int tau) {
        if (tau >= sampleRate / 500) return minimum (cmndf, tau);
        const double refined = refine (difference, tau);
        const double normaliser = difference[tau] / std::max (1e-15, cmndf[tau]);
        return std::max (0.0, fractionalValue (difference, refined) / std::max (1e-15, normaliser));
    };
    int candidate = -1;
    double score = 1.0;
    for (int tau = minTau; tau <= maxTau; ++tau)
    {
        if (cmndf[tau] > cmndf[tau - 1] || cmndf[tau] >= cmndf[tau + 1])
            continue;
        const double value = scoreAt (tau);
        if (value < score)
        {
            candidate = tau;
            score = value;
        }
        if (value < threshold)
            break;
    }
    // A global minimum alone is not evidence of pitch (noise always has one).
    if (candidate < 0 || score > 0.20)
        return std::nullopt;

    // Resolve a strong second/third harmonic only when its putative fundamental
    // gives a substantially better match; never prefer an arbitrary long lag.
    const double firstPeriod = candidate < sampleRate / 500 ? refine (difference, candidate) : interpolate (difference, candidate);
    for (int multiple = 2; multiple <= 3; ++multiple)
    {
        const int centre = (int)std::lround (firstPeriod * multiple);
        const int radius = std::max (2, (int)std::ceil (firstPeriod * multiple * 0.02));
        if (centre + radius > maxTau)
            break;
        int best = centre;
        for (int tau = centre - radius; tau <= centre + radius; ++tau)
            if (cmndf[tau] < cmndf[best]) best = tau;
        if (best == centre - radius || best == centre + radius)
            continue;
        const double value = scoreAt (best);
        if (score > 0.01 && value < score * 0.2)
        {
            candidate = best;
            score = value;
            break;
        }
    }

    const double period = candidate < sampleRate / 500 ? refine (difference, candidate) : interpolate (difference, candidate);
    // Fit several repetitions through the origin. Longer lags reduce fractional
    // sample bias on the top piano octave without quantising to musical notes.
    double lagSum = period, weight = 1.0;
    const int repetitions = std::min (8, (int)(maxTau / period));
    for (int multiple = 2; multiple <= repetitions; ++multiple)
    {
        const int centre = (int)std::lround (period * multiple);
        const int radius = std::max (1, (int)std::ceil (period * 0.03));
        const int low = std::max (2, centre - radius);
        const int high = std::min (maxTau, centre + radius);
        if (low >= high) break;
        int best = low;
        for (int tau = low + 1; tau <= high; ++tau)
            if (difference[tau] < difference[best]) best = tau;
        if (best == low || best == high || minimum (cmndf, best) > score + 0.03)
            continue;
        lagSum += multiple * interpolate (difference, best);
        weight += multiple * multiple;
    }
    const double refined = lagSum / weight;
    return Result { (float)refined, candidate, (float)(sampleRate / refined),
                    (float)score, (float)(1.0 - score) };
}
