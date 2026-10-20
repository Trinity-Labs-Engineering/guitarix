// Houston telemetry. All methods run on ordinary worker threads, never JACK RT.
#pragma once

#include "yin/PitchTracker.h"
#include <json.hpp>
#include <array>
#include <chrono>
#include <string>

namespace gx_houston {
using Json = nlohmann::json;

struct Meter {
    std::array<float, 2> peak{}, rms{};
    std::array<bool, 2> clipped{};
    void process(const float* left, const float* right, unsigned frames, unsigned rate);
    void reset(bool levels, bool clipping);
    Json snapshot(bool leftConnected, bool rightConnected) const;
};

class Analysis {
public:
    Analysis();
    void prepare(unsigned rate);
    void enable(bool enabled);
    void append(const float* samples, unsigned count, uint64_t end);
    void analyse();
    void expire();
    Json tuner() const;
    Json action(const std::string& action, const Json& params);
    unsigned rate = 48000;
    unsigned backlogSkips = 0;
    Meter pre, post;
private:
    Json result(const PitchTracker::Frame& frame) const;
    Json idle() const;
    Json targets(double hz, bool detected) const;
    PitchTracker tracker;
    std::vector<float> ring, frame;
    uint64_t written = 0, nextEnd = 0, validStart = 0;
    unsigned hop = 960;
    bool enabled = false;
    double a4 = 440.0;
    PitchTracker::Frame lastFrame;
    std::chrono::steady_clock::time_point lastAudio;
};
}
