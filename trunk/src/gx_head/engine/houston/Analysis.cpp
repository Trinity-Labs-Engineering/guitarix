#include "Analysis.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace gx_houston {
namespace {
double rounded(double value, double scale) { return std::floor(value * scale + 0.5) / scale; }
double frequency(int midi, double a4) { return a4 * std::pow(2.0, (midi - 69) / 12.0); }
double cents(double hz, double reference) { return 1200.0 * std::log2(hz / reference); }
double db(double amplitude) { return amplitude > 0 ? std::max(-120.0, 20 * std::log10(amplitude)) : -120.0; }
const char* names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
}

void Meter::process(const float* left, const float* right, unsigned frames, unsigned rate) {
    if (!frames) return;
    const double duration = double(frames) / std::max(1u, rate);
    const float peakRelease = std::exp(-duration / 0.35);
    const float rmsRelease = std::exp(-duration / 0.45);
    const float* channels[] = {left, right};
    for (unsigned ch = 0; ch < 2; ++ch) {
        double sum = 0;
        float maximum = 0;
        for (unsigned i = 0; i < frames; ++i) {
            const float value = channels[ch] ? channels[ch][i] : 0;
            // A corrupt source must not poison all subsequent readings.
            if (!std::isfinite(value)) continue;
            maximum = std::max(maximum, std::abs(value));
            sum += double(value) * value;
        }
        peak[ch] = std::max(maximum, peak[ch] * peakRelease);
        rms[ch] = std::max(float(std::sqrt(sum / frames)), rms[ch] * rmsRelease);
        clipped[ch] = clipped[ch] || maximum >= 0.999f;
    }
}

void Meter::reset(bool levels, bool clipping) {
    if (levels) { peak = {}; rms = {}; }
    if (clipping) clipped = {};
}

Json Meter::snapshot(bool left, bool right) const {
    const float peakMax = std::max(peak[0], peak[1]), rmsMax = std::max(rms[0], rms[1]);
    return {{"available", left || right}, {"stereo_available", left && right},
        {"peak_l", rounded(peak[0], 1e6)}, {"peak_r", rounded(peak[1], 1e6)},
        {"rms_l", rounded(rms[0], 1e6)}, {"rms_r", rounded(rms[1], 1e6)},
        {"peak_max", rounded(peakMax, 1e6)}, {"rms_max", rounded(rmsMax, 1e6)},
        {"db", rounded(db(peakMax), 1e3)}, {"db_peak", rounded(db(peakMax), 1e3)},
        {"db_rms", rounded(db(rmsMax), 1e3)}, {"clipped_l", clipped[0]},
        {"clipped_r", clipped[1]}, {"clip_threshold", 0.999}};
}

Analysis::Analysis() { prepare(rate); }

void Analysis::prepare(unsigned newRate) {
    rate = std::max(1u, newRate);
    tracker.prepare(rate);
    hop = std::max(1, int(std::lround(rate * PitchTracker::hopSeconds)));
    ring.assign(std::max(16384u, rate * 2), 0);
    frame.resize(tracker.frameLength());
    written = validStart = 0;
    nextEnd = frame.size();
    lastFrame = {};
    backlogSkips = 0;
    lastAudio = std::chrono::steady_clock::now();
    pre.reset(true, true);
    post.reset(true, true);
}

void Analysis::enable(bool value) {
    if (enabled == value) return;
    enabled = value;
    tracker.reset();
    lastFrame = {};
    validStart = written;
    nextEnd = written + frame.size();
}

void Analysis::append(const float* samples, unsigned count, uint64_t end) {
    if (!enabled || !count || end < count) return;
    const uint64_t start = end - count;
    if (start != written) {
        // Overflow/gaps must never splice unrelated audio into a pitch frame.
        tracker.reset();
        lastFrame = {};
        validStart = start;
        nextEnd = start + frame.size();
        ++backlogSkips;
    }
    for (unsigned i = 0; i < count; ++i) ring[(start + i) % ring.size()] = samples[i];
    written = end;
    lastAudio = std::chrono::steady_clock::now();
}

void Analysis::analyse() {
    if (!enabled || written < nextEnd) return;
    const auto end = written - (written - validStart - frame.size()) % hop;
    if (end - nextEnd >= 2 * hop) { tracker.reset(); ++backlogSkips; }
    const auto start = end - frame.size();
    if (start < validStart || written - start > ring.size()) return;
    for (size_t i = 0; i < frame.size(); ++i) frame[i] = ring[(start + i) % ring.size()];
    lastFrame = tracker.process(frame.data(), frame.size(), end);
    nextEnd = end + hop;
}

void Analysis::expire() {
    if (std::chrono::steady_clock::now() - lastAudio > std::chrono::milliseconds(150)) {
        tracker.reset();
        lastFrame = {};
    }
}

Json Analysis::idle() const {
    return {{"detected", false}, {"tracking_confident", false}, {"frequency_hz", 0.0},
        {"raw_frequency_hz", 0.0}, {"note_name", "--"}, {"note_label", "--"},
        {"octave", nullptr}, {"midi_note", nullptr}, {"target_frequency_hz", 0.0},
        {"cents", 0.0}, {"direction", "idle"}, {"in_tune", false}, {"clarity", 0.0},
        {"selected_cmndf", 1.0}, {"period_frames", 0.0}, {"period_seconds", 0.0},
        {"rms_level", 0.0}, {"peak_level", 0.0}, {"status", "Listening for a stable note"}};
}

Json Analysis::result(const PitchTracker::Frame& f) const {
    Json out = idle();
    out["rms_level"] = rounded(f.rms, 1e6);
    out["peak_level"] = rounded(f.peak, 1e6);
    out["raw_frequency_hz"] = f.rawFrequencyHz;
    if (!f.pitch) return out;
    const auto& y = *f.pitch;
    const double hz = y.frequencyHz;
    const int midi = int(std::round(69.0 + 12.0 * std::log2(hz / a4)));
    const double nearest = frequency(midi, a4), error = cents(hz, nearest);
    const bool inTune = std::abs(error) <= 2.5;
    const std::string label = names[(midi % 12 + 12) % 12];
    out.update({{"detected", true}, {"tracking_confident", true},
        {"frequency_hz", rounded(hz, 1e4)}, {"note_name", label + std::to_string(midi / 12 - 1)},
        {"note_label", label}, {"octave", midi / 12 - 1}, {"midi_note", midi},
        {"target_frequency_hz", rounded(nearest, 1e4)}, {"cents", rounded(error, 1e3)},
        {"direction", inTune ? "in_tune" : (error > 0 ? "sharp" : "flat")}, {"in_tune", inTune},
        {"clarity", rounded(y.clarity, 1e6)}, {"selected_cmndf", rounded(y.cmndf, 1e6)},
        {"period_frames", rounded(y.tau, 1e6)}, {"period_seconds", rounded(double(y.tau) / rate, 1e6)},
        {"status", inTune ? "In tune" : (error > 0 ? "Too sharp" : "Too flat")}});
    return out;
}

Json Analysis::targets(double hz, bool detected) const {
    const int midis[] = {40,45,50,55,59,64};
    const char* ids[] = {"string-low-e","string-a","string-d","string-g","string-b","string-high-e"};
    const char* labels[] = {"Low E","A","D","G","B","High E"};
    int nearest = -1;
    double distance = std::numeric_limits<double>::infinity();
    if (detected && hz > 0)
        for (int i = 0; i < 6; ++i) {
            const double d = std::abs(cents(hz, frequency(midis[i], a4)));
            if (d < distance) { distance = d; nearest = i; }
        }
    Json out = Json::array();
    for (int i = 0; i < 6; ++i) {
        const double target = frequency(midis[i], a4);
        const double error = detected && hz > 0 ? cents(hz, target) : 0;
        out.push_back({{"id", ids[i]}, {"label", labels[i]},
            {"note_name", std::string(names[midis[i] % 12]) + std::to_string(midis[i] / 12 - 1)},
            {"frequency_hz", rounded(target, 1e4)}, {"cents_off", rounded(error, 1e3)},
            {"nearest", nearest == i}, {"in_tune", detected && std::abs(error) <= 2.5}});
    }
    return out;
}

Json Analysis::tuner() const {
    auto out = result(lastFrame);
    out.update({{"a4_frequency", a4}, {"threshold", 0.10},
        {"search_range_hz", {{"min", PitchTracker::minFrequency}, {"max", PitchTracker::maxFrequency}}},
        {"tuning_window_cents", 2.5}, {"analysis_window_ms", rounded(double(frame.size()) / rate, 1e6) * 1000},
        {"analysis_hop_ms", rounded(double(hop) / rate, 1e6) * 1000},
        {"pitch_smoothing_window_ms", 80}, {"smoothing_window_ms", 80},
        {"level_gate_open_rms", PitchTracker::openLevel}, {"level_gate_close_rms", PitchTracker::closeLevel},
        {"best_local_window_ms", 0.0}, {"analysis_enabled", enabled}});
    out["string_targets"] = targets(out["frequency_hz"], out["detected"]);
    return out;
}

Json Analysis::action(const std::string& name, const Json& params) {
    if (name == "reference.set" || name == "reference.reset") {
        double hz = 440;
        if (name == "reference.set") {
            if (!params.contains("frequency") || !params["frequency"].is_number())
                throw std::invalid_argument("'frequency' must be a number between 400 and 480.");
            hz = params["frequency"];
            if (!std::isfinite(hz) || hz < 400 || hz > 480)
                throw std::invalid_argument("'frequency' must be between 400 and 480.");
        }
        a4 = hz;
        return {{"a4_frequency", a4}};
    }
    if (name == "tracking.set") {
        // Houston sends this bootstrap; tracking remains the original fixed 80 ms.
        return {{"smoothing_window_ms", 80}};
    }
    throw std::invalid_argument("Unknown action '" + name + "'.");
}
}
