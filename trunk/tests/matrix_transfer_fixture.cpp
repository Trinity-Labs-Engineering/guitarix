#include <cassert>
#include <iostream>
#include <string>
#include <vector>

static std::vector<std::string> events;
static void gx_print_warning(const char*, const char*) {}
struct ProcessingChainBase { enum { ramp_mode_off, ramp_mode_down_dead }; };
struct SceneAudioCycleStatus {
    bool complete;
    bool finished() const { return complete; }
};
struct PluginList {
    bool source_changed = false;
    bool state_changed = false;
    bool rt_scene_state_changed() { return state_changed; }
    bool matrix_sources_changed() { return source_changed; }
    bool commit_rt_scene_states() {
        events.push_back("snapshot");
        source_changed = state_changed = false;
        return true;
    }
};
struct Chain {
    std::string name;
    bool next_commit_needs_ramp = true;
    int ramp = ProcessingChainBase::ramp_mode_off;
    int publications = 0;
    int get_ramp_mode() { return ramp; }
    void start_ramp_down() { ramp = ProcessingChainBase::ramp_mode_down_dead; }
    void wait_ramp_down_finished() {}
    void start_ramp_up() { ramp = ProcessingChainBase::ramp_mode_off; }
    bool commit(bool, int&) {
        events.push_back(name);
        ++publications;
        return true;
    }
};
struct ModuleSequencer {
    PluginList pluginlist;
    Chain mono_chain{"mono"}, stereo_chain{"stereo"};
    bool pending_matrix_transfer = false;
    bool callbacks_finished = true;
    int parameter = 0;
    int& get_param() { return parameter; }
    SceneAudioCycleStatus wait_scene_control_cycle(bool mono, bool stereo) {
        assert(mono && stereo);
        events.push_back("drain");
        return {callbacks_finished};
    }
    bool commit_module_lists(bool externally_muted);
};
#include "matrix_transfer_under_test.h"

int main() {
    for (bool muted : {false, true}) {
        ModuleSequencer engine;
        engine.pluginlist.source_changed = engine.pluginlist.state_changed = true;
        engine.callbacks_finished = false;
        events.clear();
        assert(!engine.commit_module_lists(muted));
        assert((events == std::vector<std::string>{"snapshot", "drain"}));
        assert(engine.pending_matrix_transfer);
        assert(engine.mono_chain.publications == 0 && engine.stereo_chain.publications == 0);
        // Failure cannot strand an interactively ramped chain in silence.
        assert(engine.mono_chain.ramp == ProcessingChainBase::ramp_mode_off);
        assert(engine.stereo_chain.ramp == ProcessingChainBase::ramp_mode_off);

        // The published source bit already matches on retry, but the old DSP
        // callback still needs draining before the destination initialises it.
        engine.callbacks_finished = true;
        events.clear();
        assert(engine.commit_module_lists(muted));
        assert((events == std::vector<std::string>{"snapshot", "drain", "mono", "stereo"}));
        assert(!engine.pending_matrix_transfer);
        events.clear();
        assert(engine.commit_module_lists(muted));
        assert((events == std::vector<std::string>{"snapshot", "mono", "stereo"}));
    }
    std::cout << "matrix-transfer-ok\n";
}
