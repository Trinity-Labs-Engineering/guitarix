#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
#include "gx_scene_input_history.h"

using gx_engine::SceneInputHistory;
constexpr unsigned int PGNI_DYN_POSITION = 0x10000;
constexpr unsigned int PGNI_RESIDENT_TRACK_BYPASS = 0x400000;
#define __rt_func
struct PluginDef {
    const char* id;
    unsigned int flags;
    void (*clear_state)(PluginDef*) = nullptr;
    unsigned int calls = 0, resets = 0;
    std::vector<float> observed;
};
struct Plugin { bool on = true; bool get_rt_on_off() const { return on; } };
struct monochain_data {
    void (*func)(int, float*, float*, PluginDef*);
    PluginDef* plugin;
    Plugin* owner;
};
#include "scene_prefix_status_under_test.h"
class MonoModuleChain {
public:
    SceneInputHistory scene_input_history;
    std::uintptr_t history_upstream_signature = 0;
    std::vector<monochain_data> nodes;
    unsigned int input_cursor = 0;
    unsigned int silent_callbacks = 0;
    bool stopped = false, fail_wait = false;
    enum RampMode { ramp_mode_down_dead, ramp_mode_down, ramp_mode_up_dead, ramp_mode_up, ramp_mode_off };
    int steps_up_dead = 0, steps_up = 1, steps_down = 1;
    RampMode get_ramp_mode() const { return ramp_mode_off; }
    int get_ramp_value() const { return 1; }
    void try_set_ramp_mode(RampMode, RampMode, int, int) {}
    bool is_stopped() const { return stopped; }
    bool is_down_dead() const { return false; }
    monochain_data* get_rt_chain() { return nodes.data(); }
    void set_latch() {}
    bool wait_rt_finished_until(const timespec&) {
        if (fail_wait) return false;
        callback();
        return true;
    }
    void callback() {
        float input[32], output[32];
        for (float& value : input) value = static_cast<float>(++input_cursor);
        process(32, input, output);
        if (std::all_of(output, output + 32, [](float value) { return value == 0; }))
            ++silent_callbacks;
    }
    bool preroll_scene_prefix(bool, ScenePrefixPrerollStatus&);
    void process(int count, float* input, float* output);
};
#include "scene_prefix_under_test.h"

static void reset(PluginDef* plugin) { ++plugin->resets; }
static void render(int count, float* input, float* output, PluginDef* plugin) {
    ++plugin->calls;
    plugin->observed.insert(plugin->observed.end(), input, input + count);
    for (int i = 0; i < count; ++i)
        output[i] = input[i] + (std::strcmp(plugin->id, "nam") == 0 ? 1.0f : 0.0f);
}
int main() {
    MonoModuleChain chain;
    chain.scene_input_history.set_samplerate(1000);
    Plugin nam_owner{false}, hog_owner{false}, amp_owner{true};
    PluginDef nam{"nam", PGNI_DYN_POSITION, reset};
    PluginDef hog{"hogsfoot", PGNI_DYN_POSITION, reset};
    PluginDef amp{"mnam", PGNI_DYN_POSITION, reset};
    chain.nodes = {{render, &nam, &nam_owner}, {render, &hog, &hog_owner},
                   {render, &amp, &amp_owner}, {nullptr, nullptr, nullptr}};
    // A first prepared activation has real input history even while NAM and
    // the slow filter have never been audible. No calibration tone is needed.
    for (int i = 0; i < 20; ++i) chain.callback();
    assert(nam.calls == 0 && hog.calls == 0);
    nam_owner.on = hog_owner.on = true;
    ScenePrefixPrerollStatus status;
    assert(chain.preroll_scene_prefix(false, status));
    assert(!status.attempted && std::strcmp(status.skip_reason, "no-external-mute") == 0);
    const unsigned int old_amp_calls = amp.calls;
    assert(chain.preroll_scene_prefix(true, status));
    assert(status.attempted && status.history_frames == 512 && status.catchup_frames == 32);
    assert(chain.silent_callbacks == 1);
    assert(nam.resets == 0 && hog.resets == 1);
    assert(amp.calls == old_amp_calls + 1); // Suffix only runs on resumed live input.
    // Exactly chronological real samples: old window, queued live input, then
    // current callback, with no duplicated/missed input at the ownership cut.
    assert(nam.observed.size() == 512 + 32 + 32);
    for (unsigned int i = 0; i < nam.observed.size(); ++i)
        assert(nam.observed[i] == static_cast<float>(129 + i));
    for (unsigned int i = 0; i < hog.observed.size(); ++i)
        assert(hog.observed[i] == static_cast<float>(130 + i));

    // An unsupported time effect before the target filter must prevent replay.
    PluginDef delay{"echo", PGNI_DYN_POSITION, reset};
    Plugin delay_owner;
    chain.nodes.insert(chain.nodes.begin() + 1, {render, &delay, &delay_owner});
    assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
    chain.nodes.erase(chain.nodes.begin() + 1);

    // The historical capture point cannot move across a changing upstream DSP.
    chain.nodes.insert(chain.nodes.begin(), {render, &delay, &delay_owner});
    chain.callback();
    assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
    assert(std::strcmp(status.skip_reason, "scene-varies-before-nam") == 0);
    chain.nodes.erase(chain.nodes.begin());
    assert(chain.preroll_scene_prefix(true, status));
    assert(status.history_frames == 0); // No invented history after invalid input.

    // Even an allowed parameterless upstream stage changes the capture source
    // when toggled. The first paused callback must invalidate the old window.
    for (int i = 0; i < 4; ++i) chain.callback();
    PluginDef dcblocker{"dcblocker", 0, reset};
    Plugin dc_owner;
    chain.nodes.insert(chain.nodes.begin(), {render, &dcblocker, &dc_owner});
    assert(chain.preroll_scene_prefix(true, status));
    assert(status.history_frames == 0);
    chain.nodes.erase(chain.nodes.begin());

    // A gap with no NAM must not retain samples from the previous song.
    for (int i = 0; i < 4; ++i) chain.callback();
    const auto old_nam = chain.nodes.front();
    chain.nodes.erase(chain.nodes.begin());
    chain.callback();
    chain.nodes.insert(chain.nodes.begin(), old_nam);
    assert(chain.preroll_scene_prefix(true, status));
    assert(status.history_frames == 0);

    // Explicit whole-chain reset invalidates the source history at the next
    // callback without a control thread racing the RT-owned ring indices.
    for (int i = 0; i < 4; ++i) chain.callback();
    chain.scene_input_history.invalidate_control();
    assert(chain.preroll_scene_prefix(true, status));
    assert(status.history_frames == 0);

    // A callback ownership failure never reports a successful replay.
    chain.fail_wait = true;
    assert(!chain.preroll_scene_prefix(true, status));
    assert(chain.scene_input_history.state() == SceneInputHistory::abort_requested);
    chain.fail_wait = false;
    chain.callback();
    assert(chain.scene_input_history.state() == SceneInputHistory::idle);
    std::cout << "scene-prefix-preroll-ok\n";
}
