#include "PitchTracker.h"

#include <algorithm>
#include <cmath>
#include <vector>

void PitchTracker::prepare (int sampleRate)
{
    rate = std::max (8000, sampleRate);
    frameSize = 3 * (int)std::ceil (rate / minFrequency) + 3;
    reset();
}

void PitchTracker::reset()
{
    gateOpen = false;
    previousEnd = gateOpenedAt = 0;
    smoothedLogHz = pendingLogHz = 0.0;
    pendingCount = 0;
    history.clear();
}

PitchTracker::Frame PitchTracker::process (const float* samples, int count, int64_t endSample)
{
    Frame frame;
    if (! samples || count != frameSize || endSample <= previousEnd)
    {
        reset();
        return frame;
    }
    const double elapsed = previousEnd > 0 ? (double)(endSample - previousEnd) / rate : hopSeconds;
    if (elapsed > 0.10)
        reset();
    previousEnd = endSample;

    // Measure the newest 40 ms, independently of the 120 ms pitch window.
    // Subtract DC so a biased input cannot open the gate. More than one A0
    // period keeps the bass waveform's zero crossings from chattering it.
    const int tail = std::min (count, (int)std::ceil (rate * 0.040));
    double sum = 0.0, sumSq = 0.0;
    for (int i = count - tail; i < count; ++i)
    {
        if (! std::isfinite (samples[i]))
        {
            reset();
            return frame;
        }
        sum += samples[i];
        sumSq += (double)samples[i] * samples[i];
        frame.peak = std::max (frame.peak, std::abs (samples[i]));
    }
    frame.rms = (float)std::sqrt (std::max (0.0, sumSq / tail - std::pow (sum / tail, 2)));
    if (frame.rms < (gateOpen ? closeLevel : openLevel))
    {
        reset();
        return frame;
    }
    if (! gateOpen)
        gateOpenedAt = endSample;
    gateOpen = true;
    // Do not compare the attack to the silence preceding it. Both halves of
    // the YIN window need voiced audio before the first displayed estimate.
    if (endSample - gateOpenedAt < frameSize - tail / 2)
        return frame;

    auto result = yin.run (samples, count, rate,
                           std::max (2, (int)std::floor (rate / maxFrequency)),
                           (int)std::ceil (rate / minFrequency));
    if (! result || result->frequencyHz < minFrequency || result->frequencyHz > maxFrequency)
    {
        // Invalid audio must actively clear the display, including a noisy
        // release. There is no historical "best" candidate to leave hanging.
        history.clear();
        smoothedLogHz = pendingLogHz = 0.0;
        pendingCount = 0;
        return frame;
    }
    frame.rawFrequencyHz = result->frequencyHz;
    const double logHz = std::log2 ((double)result->frequencyHz);
    const bool newNote = history.empty() || std::abs (1200.0 * (logHz - smoothedLogHz)) > 50.0;
    if (newNote)
    {
        if (pendingCount == 0 || std::abs (1200.0 * (logHz - pendingLogHz)) > 25.0)
        {
            pendingLogHz = logHz;
            pendingCount = 1;
        }
        else
        {
            pendingLogHz += (logHz - pendingLogHz) / ++pendingCount;
        }
        // Two agreeing frames acquire a note; three confirm a change. While
        // changing, clear rather than display an invented intermediate pitch.
        const int required = history.empty() ? 2 : 3;
        if (pendingCount < required)
            return frame;
        history.clear();
        smoothedLogHz = pendingLogHz;
    }
    pendingCount = 0;
    history.push_back (logHz);
    while (history.size() > 5)
        history.pop_front();
    std::vector<double> sorted (history.begin(), history.end());
    std::sort (sorted.begin(), sorted.end());
    const auto middle = sorted.size() / 2;
    const double median = sorted.size() % 2 ? sorted[middle]
                                            : 0.5 * (sorted[middle - 1] + sorted[middle]);
    const double alpha = 1.0 - std::exp (-std::min (elapsed, 0.06) / smoothingSeconds);
    smoothedLogHz += alpha * (median - smoothedLogHz);
    result->frequencyHz = (float)std::exp2 (smoothedLogHz);
    result->tau = rate / result->frequencyHz;
    result->integerTau = (int)std::lround (result->tau);
    frame.pitch = result;
    return frame;
}
