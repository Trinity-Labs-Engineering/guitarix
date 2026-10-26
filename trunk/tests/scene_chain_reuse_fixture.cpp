// Exercise the production chain-pointer template without a JACK server or UI.
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <list>
#include <set>
#include <vector>

using std::list;
using std::set;

namespace gx_system {
template <class T> void atomic_set(T* value, T replacement) { *value = replacement; }
template <class T> T atomic_get(T value) { return value; }
}

struct ParamMap {};
struct PluginDef {
    const char* id;
    unsigned int flags = 0;
    int (*activate_plugin)(bool, PluginDef*) = nullptr;
    void (*clear_state)(PluginDef*) = nullptr;
    unsigned int initialization_count = 0;
};
struct Plugin {
    PluginDef definition;
    bool on = true;
    bool rt_on = true;
    PluginDef* get_pdef() { return &definition; }
    bool get_on_off() const { return on; }
    bool get_rt_on_off() const { return rt_on; }
    void set_on_off(bool value) { on = value; }
    void commit_rt_scene_state() { rt_on = on; }
};
struct stringcomp {
    bool operator()(const char* a, const char* b) const { return std::strcmp(a, b) < 0; }
};
const unsigned int PGN_SNOOP = 1;
const unsigned int PGNI_RESIDENT_TRACK_BYPASS = 0x400000;

class ProcessingChainBase {
protected:
    list<Plugin*> modules;
    list<Plugin*> to_initialize;
    list<Plugin*> to_release;
    bool pending_module_list = false;
public:
    unsigned int publications = 0;
    unsigned int lifetime_waits = 0;
    bool next_commit_needs_ramp = false;
    enum RampMode { ramp_mode_down_dead, ramp_mode_off };
    unsigned int ramps_down = 0, ramps_up = 0, ramp_waits = 0;
    RampMode get_ramp_mode() const { return ramp_mode_off; }
    void start_ramp_down() { ++ramps_down; }
    void start_ramp_up() { ++ramps_up; }
    void wait_ramp_down_finished() { ++ramp_waits; }
    bool set_plugin_list(const list<Plugin*>& plugins);
    void clear_module_states();
    bool has_pending_module_list() const { return pending_module_list; }
    void set_latch() { ++publications; }
    void wait_latch() { ++lifetime_waits; }
    bool check_release() { return !to_release.empty(); }
    void release() { to_release.clear(); }
};

// Generated directly from gx_modulesequencer.h and gx_engine_audio.cpp by
// test_scene_chain_reuse.py: real staging/comparison/publication code.
#include "scene_chain_under_test.h"

struct ChainNode {
    void (*func)() = nullptr;
    Plugin* owner = nullptr;
};
static void process_stub() {}
template <> inline ChainNode ThreadSafeChainPointer<ChainNode>::get_audio(Plugin* p) {
    return ChainNode{process_stub, p};
}
class TestChain : public ThreadSafeChainPointer<ChainNode> {
public:
    ChainNode* published() { return get_rt_chain(); }
};

struct PluginListFixture {
    std::vector<Plugin*> plugins;
    bool commit_rt_scene_states() {
        for (Plugin* plugin : plugins) plugin->commit_rt_scene_state();
        return true;
    }
};
class ModuleSequencer {
    ParamMap parameters;
public:
    TestChain mono_chain, stereo_chain;
    PluginListFixture pluginlist;
    ParamMap& get_param() { return parameters; }
    bool commit_module_lists(bool externally_muted);
};
static void gx_print_warning(const char*, const char*) {}
#include "scene_commit_under_test.h"

static void initialize(PluginDef* plugin) { ++plugin->initialization_count; }
static int reject_activation(bool, PluginDef*) { return 1; }
static int accept_activation(bool, PluginDef* plugin) {
    ++plugin->initialization_count;
    return 0;
}

int main() {
    ParamMap parameters;
    TestChain chain;
    ChainNode* empty = chain.published();
    assert(chain.commit(false, parameters));
    assert(chain.published() == empty && chain.publications == 0);

    Plugin first{{"first"}};
    first.definition.clear_state = initialize;
    assert(chain.set_plugin_list({&first}));
    assert(chain.commit(true, parameters));
    ChainNode* resident = chain.published();
    assert(resident != empty && resident[0].owner == &first);
    assert(first.definition.initialization_count == 1 && chain.publications == 1);

    // Repeated bypass changes leave the list and its lifetime latch alone;
    // the node still refers to the same live per-plugin bypass snapshot.
    const unsigned int previous_waits = chain.lifetime_waits;
    for (unsigned int i = 0; i < 100; ++i) {
        first.on = !first.on;
        assert(!chain.set_plugin_list({&first}));
        assert(chain.next_commit_needs_ramp);
        first.commit_rt_scene_state();
        assert(chain.commit(true, parameters));
        assert(chain.published() == resident && chain.publications == 1);
        assert(chain.published()[0].owner->on == first.on);
    }
    assert(chain.lifetime_waits == previous_waits);
    assert(first.definition.initialization_count == 1);

    Plugin second{{"second"}};
    second.definition.clear_state = initialize;
    assert(chain.set_plugin_list({&first, &second}));
    assert(chain.commit(true, parameters));
    assert(chain.publications == 2 && chain.published()[1].owner == &second);
    assert(first.definition.initialization_count == 1);
    assert(second.definition.initialization_count == 1);

    // Reordering is a real topology change, even without new processors.
    assert(chain.set_plugin_list({&second, &first}));
    assert(chain.commit(true, parameters));
    assert(chain.publications == 3 && chain.published()[0].owner == &second);
    assert(second.definition.initialization_count == 1);

    Plugin rejected{{"rejected"}};
    rejected.definition.activate_plugin = reject_activation;
    assert(chain.set_plugin_list({&second, &first, &rejected}));
    assert(!chain.commit(true, parameters));
    assert(!rejected.on && !chain.published()[2].func);
    assert(chain.has_pending_module_list());

    // A retry can contain the same desired list and no changed bypass flags.
    // Retry the missing node's activation; never silently reuse the partial
    // publication, and never reset processors already published successfully.
    const unsigned int failed_waits = chain.lifetime_waits;
    assert(!chain.set_plugin_list({&second, &first, &rejected}));
    assert(chain.has_pending_module_list());
    assert(chain.lifetime_waits > failed_waits && chain.next_commit_needs_ramp);
    rejected.definition.activate_plugin = accept_activation;
    assert(chain.commit(true, parameters));
    assert(chain.published()[2].owner == &rejected);
    assert(!chain.has_pending_module_list());
    assert(rejected.definition.initialization_count == 1);
    assert(first.definition.initialization_count == 1);
    assert(second.definition.initialization_count == 1);
    assert(chain.publications == 5);

    // A resident lazy processor's activation may do no work. An explicit
    // chain reset must still clear its history, while a failed activation
    // must never call clear_state against absent storage.
    second.definition.activate_plugin = [](bool, PluginDef*) { return 0; };
    rejected.definition.activate_plugin = reject_activation;
    rejected.definition.clear_state = initialize;
    chain.clear_module_states();
    assert(first.definition.initialization_count == 2);
    assert(second.definition.initialization_count == 2);
    assert(rejected.definition.initialization_count == 1);

    assert(chain.set_plugin_list({}));
    assert(chain.commit(true, parameters));
    assert(!chain.published()[0].func && chain.publications == 6);
    assert(chain.commit(true, parameters));
    assert(chain.publications == 6);

    // A growing resident union publishes and initializes only the new
    // bypassed DSP. Neither output ramps and active filter history survives.
    ModuleSequencer seq;
    Plugin a{{"a"}}, b{{"b"}}, spare{{"spare"}, false, false};
    a.definition.clear_state = b.definition.clear_state =
        spare.definition.clear_state = initialize;
    seq.pluginlist.plugins = {&a, &b, &spare};
    assert(seq.mono_chain.set_plugin_list({&a, &b}));
    assert(seq.commit_module_lists(true));
    assert(seq.mono_chain.set_plugin_list({&a, &spare, &b}));
    assert(!seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 0 && seq.stereo_chain.ramps_down == 0);
    assert(a.definition.initialization_count == 1 && b.definition.initialization_count == 1);
    assert(spare.definition.initialization_count == 1);
    assert(seq.mono_chain.published()[1].owner == &spare);

    // Moving/removing bypassed ordinary residents still leaves the active
    // sequence unchanged. Re-enabling one is an audible change, including
    // when the physical list does not change.
    assert(seq.mono_chain.set_plugin_list({&spare, &a, &b}));
    assert(!seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    spare.on = true;
    ChainNode* before_enable = seq.mono_chain.published();
    assert(!seq.mono_chain.set_plugin_list({&spare, &a, &b}));
    assert(seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 1 && seq.stereo_chain.ramps_down == 1);
    assert(seq.mono_chain.ramps_up == 1 && seq.stereo_chain.ramps_up == 1);
    assert(seq.mono_chain.published() == before_enable && spare.rt_on);
    spare.on = false;
    assert(!seq.mono_chain.set_plugin_list({&spare, &a, &b}));
    assert(seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(!spare.rt_on && seq.mono_chain.ramps_down == 2);
    assert(seq.mono_chain.set_plugin_list({&a, &b}));
    assert(!seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 2);

    // Active reorder/removal and bypass-tracking additions retain ramps.
    assert(seq.mono_chain.set_plugin_list({&b, &a}));
    assert(seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 3);
    assert(seq.mono_chain.set_plugin_list({&b}));
    assert(seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 4);
    spare.definition.flags = PGNI_RESIDENT_TRACK_BYPASS;
    assert(seq.mono_chain.set_plugin_list({&spare, &b}));
    assert(seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 5);
    spare.on = true;
    assert(!seq.mono_chain.set_plugin_list({&spare, &b}));
    assert(seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 6 && spare.rt_on);
    spare.on = false;
    assert(!seq.mono_chain.set_plugin_list({&spare, &b}));
    assert(seq.mono_chain.next_commit_needs_ramp);
    assert(seq.commit_module_lists(false));
    assert(seq.mono_chain.ramps_down == 7 && !spare.rt_on);

    // Trailing/empty observer lists are safe; the old comparison could
    // dereference end() after advancing past its last SNOOP node.
    Plugin observer{{"observer", PGN_SNOOP}};
    bool needs_ramp = true;
    assert(!lists_equal({&observer}, {}, &needs_ramp) && !needs_ramp);
    assert(!lists_equal({}, {&observer}, &needs_ramp) && !needs_ramp);
    assert(!lists_equal({&b, &observer}, {&b}, &needs_ramp) && !needs_ramp);
    std::cout << "scene-chain-reuse-ok\n";
}
