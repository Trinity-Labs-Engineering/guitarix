// Run the production coordinator against deterministic callback phases. A
// biased amp followed by a DC blocker makes even one transparent block audible.
#include <cassert>
#include <chrono>
#include <cmath>
#include <initializer_list>
#include <thread>
struct Dsp {
    bool ready = true, pending = false;
    int requests = 0, presets = 0, suspensions = 0;
    float previous = 0.1f, filtered = 0, peak = 0;
    void request_scene_smoother_snap() { pending = true; ++requests; }
    bool is_scene_smoother_snap_pending() const { return pending; }
    bool suspend_scene_smoother_for_control() { ++suspensions; ready = false; return true; }
    void finish_scene_smoother_control(bool restore, bool preset) {
        ready = restore; if (preset) { ++presets; pending = false; }
    }
    void process(bool consume) {
        if (pending && consume) { ++presets; pending = false; }
        const float input = ready ? 0.1f : 0;
        filtered = input - previous + 0.99f * filtered;
        previous = input;
        peak = std::fmax(peak, std::fabs(filtered));
    }
};
struct GxEngine {
    Dsp scene_outputlevel, neural_amp, sneural_amp, mneural_amp, neural_amp_second;
    bool late = false, stalled = false;
    unsigned waits = 0, max_budget = 0;
    bool preset_muted_scene_smoothers(bool, bool, bool, bool, bool = false);
    void wait_scene_audio_cycle_with_budget(bool mono, bool stereo, bool observe, unsigned budget) {
        assert(!observe && (mono || stereo));
        assert(budget > 0 && budget <= 50000);
        ++waits; if (budget > max_budget) max_budget = budget;
        if (stalled) { std::this_thread::sleep_for(std::chrono::microseconds(budget)); return; }
        const bool consume = !late || waits > 1;
        for (auto* dsp : {&neural_amp, &sneural_amp, &mneural_amp, &neural_amp_second})
            dsp->process(consume && mono);
        scene_outputlevel.process(consume && stereo);
    }
};
#include "scene_smoother_under_test.h"
int main() {
    GxEngine engine;
    assert(engine.preset_muted_scene_smoothers(false, false, false, false));
    assert(engine.waits == 0);
    for (int i = 0; i < 1000; ++i)
        assert(engine.preset_muted_scene_smoothers(true, true, true, true, true));
    assert(engine.waits == 1000);
    for (auto* dsp : {&engine.scene_outputlevel, &engine.neural_amp, &engine.sneural_amp,
                     &engine.mneural_amp, &engine.neural_amp_second}) {
        assert(dsp->presets == 1000 && dsp->suspensions == 0 && dsp->peak == 0);
    }
    GxEngine late;
    late.late = true;
    assert(late.preset_muted_scene_smoothers(false, false, false, true));
    assert(late.waits == 2 && late.mneural_amp.presets == 1 && late.mneural_amp.requests == 1);
    assert(late.mneural_amp.peak == 0);
    GxEngine stalled;
    stalled.stalled = true;
    assert(!stalled.preset_muted_scene_smoothers(false, false, false, true));
    assert(stalled.mneural_amp.ready && stalled.mneural_amp.pending && stalled.mneural_amp.presets == 0);
}
