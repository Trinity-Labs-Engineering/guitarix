// Exercise the production chain-pointer template without a JACK server or UI.
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <set>

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
    bool mono_audio = true;
    bool stereo_audio = false;
};
struct BoolParameter {
    bool value = false;
    bool get_value() const { return value; }
};
struct Plugin {
    PluginDef definition;
    bool on = true;
    bool resident = false;
    int position = 0;
    BoolParameter source{}, matrix_resident{};
    BoolParameter* p_matrix_source = &source;
    BoolParameter* p_matrix_resident = &matrix_resident;
    int rt_scene_state = 0;
    bool get_on_off() const { return on; }
    bool get_scene_resident() const { return resident; }
    bool get_processing_active() const { return on || resident; }
    int get_matrix_lane() const { return 0; }
    int position_weight() const { return position; }
    int desired_rt_scene_state() const;
#include "matrix_membership_under_test.h"
    PluginDef* get_pdef() { return &definition; }
    void set_on_off(bool value) { on = value; }
    void commit_rt_scene_state() { rt_scene_state = desired_rt_scene_state(); }
};
struct stringcomp {
    bool operator()(const char* a, const char* b) const { return std::strcmp(a, b) < 0; }
};
const unsigned int PGN_SNOOP = 1;
const unsigned int PGNI_MATRIX_SPLITTER = 2;
const unsigned int PGNI_MATRIX_COMBINER = 4;
const unsigned int PGN_MODE_NORMAL = 8;
class PluginList {
public:
    using pluginmap = std::map<std::string, Plugin*>;
    pluginmap pmap;
    void ordered_processing_mono_list(list<Plugin*>& mono, int mode);
    void ordered_processing_stereo_list(list<Plugin*>& stereo, int mode);
};
#include "matrix_lists_under_test.h"

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
    bool set_plugin_list(const list<Plugin*>& plugins);
    bool has_pending_module_list() const { return pending_module_list; }
    void set_latch() { ++publications; }
    void wait_latch() { ++lifetime_waits; }
    bool check_release() { return !to_release.empty(); }
    void release() { to_release.clear(); }
    void retain_transferred_plugins(const list<Plugin*>& destination);
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

    assert(chain.set_plugin_list({}));
    assert(chain.commit(true, parameters));
    assert(!chain.published()[0].func && chain.publications == 6);
    assert(chain.commit(true, parameters));
    assert(chain.publications == 6);

    // Promoting a lifecycle-managed stereo effect into the mono loop must not
    // deactivate the same processor after its destination chain owns it.
    TestChain transfer;
    Plugin transferable{{"transfer"}};
    transferable.definition.activate_plugin = accept_activation;
    assert(transfer.set_plugin_list({&transferable}));
    assert(transfer.commit(true, parameters));
    assert(transfer.set_plugin_list({}));
    assert(transfer.check_release());
    transfer.retain_transferred_plugins({&transferable});
    assert(!transfer.check_release());
    assert(transfer.commit(true, parameters));

    // Both possible stereo sources and both boundaries stay allocated when
    // switching between explicit splits, stereo splits and a serial scene.
    Plugin chorus{{"chorus"}}, flanger{{"flanger"}};
    Plugin splitter{{"matrix_splitter"}}, combiner{{"matrix_combiner"}};
    PluginList pool;
    for (Plugin* p : {&chorus, &flanger, &splitter, &combiner}) {
        p->on = false;
        p->resident = true;
        p->definition.flags = PGN_MODE_NORMAL;
        p->definition.clear_state = initialize;
        pool.pmap[p->definition.id] = p;
    }
    splitter.definition.flags |= PGNI_MATRIX_SPLITTER;
    combiner.definition.flags |= PGNI_MATRIX_COMBINER;
    for (Plugin* p : {&chorus, &flanger}) {
        p->definition.mono_audio = false;
        p->definition.stereo_audio = true;
        p->matrix_resident.value = true;
        assert(p->matrix_source_state_changed()); // One transfer at preparation.
    }
    TestChain mono_pool, stereo_pool;
    list<Plugin*> mono_modules, stereo_modules;
    pool.ordered_processing_mono_list(mono_modules, PGN_MODE_NORMAL);
    pool.ordered_processing_stereo_list(stereo_modules, PGN_MODE_NORMAL);
    assert(mono_modules.size() == 4 && stereo_modules.empty());
    assert(mono_pool.set_plugin_list(mono_modules));
    assert(mono_pool.commit(true, parameters));
    for (auto entry : pool.pmap) entry.second->commit_rt_scene_state();
    for (int i = 0; i < 100; ++i) {
        for (int scene = 0; scene < 4; ++scene) {
            chorus.source.value = chorus.on = scene == 0;
            flanger.source.value = flanger.on = scene == 1;
            splitter.on = scene == 2;
            combiner.on = scene != 3;
            assert(!chorus.matrix_source_state_changed());
            assert(!flanger.matrix_source_state_changed());
            for (auto entry : pool.pmap) entry.second->commit_rt_scene_state();
            pool.ordered_processing_mono_list(mono_modules, PGN_MODE_NORMAL);
            pool.ordered_processing_stereo_list(stereo_modules, PGN_MODE_NORMAL);
            assert(!mono_pool.set_plugin_list(mono_modules));
            assert(!stereo_pool.set_plugin_list(stereo_modules));
            assert(mono_pool.commit(true, parameters));
            assert(stereo_pool.commit(true, parameters));
        }
    }
    for (auto entry : pool.pmap) assert(entry.second->definition.initialization_count == 1);
    assert(mono_pool.publications == 1 && stereo_pool.publications == 0);
    chorus.matrix_resident.value = false;
    assert(chorus.matrix_source_state_changed()); // Real transfers still drain callbacks.
    std::cout << "scene-chain-reuse-ok\n";
}
