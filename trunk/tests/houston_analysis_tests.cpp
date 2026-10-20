#include "houston/Analysis.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>

using namespace gx_houston;

int main() {
    Meter meter;
    const float left[] = {1.f, -1.f, .5f, -.5f};
    const float right[] = {.25f, -.25f, .25f, -.25f};
    meter.process(left, right, 4, 48000);
    assert(meter.peak[0] == 1.f && meter.peak[1] == .25f);
    assert(std::abs(meter.rms[0] - std::sqrt(.625)) < 1e-6);
    assert(meter.rms[1] == .25f && meter.clipped[0] && !meter.clipped[1]);
    const float previousRms = meter.rms[0];
    meter.process(nullptr, nullptr, 480, 48000);
    assert(std::abs(meter.peak[0] - std::exp(-.01 / .35)) < 1e-6);
    assert(std::abs(meter.rms[0] - previousRms * std::exp(-.01 / .45)) < 1e-6);
    meter.reset(false, true);
    assert(!meter.clipped[0] && meter.peak[0] > .9);
    meter.reset(true, false);
    assert(meter.peak[0] == 0 && meter.rms[0] == 0);
    assert(meter.snapshot(false, false)["db"] == -120.0);

    // Compare the complete native framing path to the original PitchTracker,
    // at all supported production rates and several JACK block sizes.
    for (unsigned rate : {44100u, 48000u, 96000u}) {
        for (unsigned block : {32u, 128u, 1024u}) {
            Analysis native;
            native.prepare(rate);
            native.enable(true);
            PitchTracker original;
            original.prepare(rate);
            const unsigned window = original.frameLength();
            const unsigned hop = std::lround(rate * PitchTracker::hopSeconds);
            std::vector<float> audio(rate);
            for (unsigned i = 0; i < audio.size(); ++i)
                audio[i] = .15 * std::sin(2 * 3.141592653589793 * 440 * i / rate);
            uint64_t next = window;
            int detected = 0;
            for (unsigned end = block; end <= audio.size(); end += block) {
                native.append(audio.data() + end - block, block, end);
                native.analyse();
                if (end < next) continue;
                const unsigned aligned = end - (end - window) % hop;
                if (aligned - next >= 2 * hop) original.reset();
                const auto frame = original.process(audio.data() + aligned - window, window, aligned);
                const auto state = native.tuner();
                assert(state["detected"] == bool(frame.pitch));
                if (frame.pitch) {
                    ++detected;
                    assert(std::abs(state["frequency_hz"].get<double>() - frame.pitch->frequencyHz) <= .000051);
                }
                assert(std::abs(state["raw_frequency_hz"].get<double>() - frame.rawFrequencyHz) < 1e-6);
                next = aligned + hop;
            }
            assert(detected > 10);
            assert(native.action("reference.set", {{"frequency", 442}})["a4_frequency"] == 442);
            assert(native.tuner()["cents"].get<double>() < -7);
            native.action("reference.reset", Json::object());
            assert(native.tuner()["a4_frequency"] == 440);
            for (const Json value : {Json("440"), Json(0), Json(481), Json(nullptr)}) {
                bool rejected = false;
                try { native.action("reference.set", {{"frequency", value}}); }
                catch (const std::invalid_argument&) { rejected = true; }
                assert(rejected);
            }
            native.enable(false);
            assert(!native.tuner()["detected"].get<bool>());
            native.enable(true);
            // A discontinuity after an overflow clears the old note.
            native.append(audio.data(), block, 3 * rate + block);
            native.analyse();
            assert(!native.tuner()["detected"].get<bool>());
        }
    }
    std::cout << "Native meter envelopes, tuner framing/parity, calibration, lifecycle and gaps passed\n";
}
