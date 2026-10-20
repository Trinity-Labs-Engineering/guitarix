#pragma once

#include "YinTracker.h"
#include <cstdint>
#include <deque>

// Complete analysis/display path, independent of JACK so the exact production
// detector, gate and smoothing can be exercised offline with generated audio.
class PitchTracker
{
public:
    static constexpr double minFrequency = 25.0;
    static constexpr double maxFrequency = 5000.0;
    static constexpr double hopSeconds = 0.020;
    static constexpr double smoothingSeconds = 0.080;
    static constexpr float openLevel = 0.006f;  // -44.4 dBFS RMS
    static constexpr float closeLevel = 0.001f; // -60.0 dBFS RMS; follow a plucked note's decay

    struct Frame
    {
        float rms { 0.f }, peak { 0.f }, rawFrequencyHz { 0.f };
        std::optional<YinTracker::Result> pitch;
    };

    void prepare (int sampleRate);
    void reset();
    int frameLength() const { return frameSize; }
    Frame process (const float* samples, int count, int64_t endSample);

private:
    YinTracker yin;
    int rate { 48000 }, frameSize { 5763 };
    bool gateOpen { false };
    int64_t previousEnd { 0 }, gateOpenedAt { 0 };
    double smoothedLogHz { 0.0 }, pendingLogHz { 0.0 };
    int pendingCount { 0 };
    std::deque<double> history;
};
