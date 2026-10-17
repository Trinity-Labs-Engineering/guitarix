// Exercise the production bank admission/ordering code with selector proxies
// which have no audio callback. Their alternative children own the DSP.
#include <cassert>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "gx_snapshot.h"
using namespace std;
constexpr unsigned PGN_ALTERNATIVE = 0x20;
struct RpcError : runtime_error {
    int code;
    RpcError(int c, const string& text): runtime_error(text), code(c) {}
};
namespace gx_engine {
constexpr unsigned PGNI_DYN_POSITION = 0x10000, PGNI_SNAPSHOT_RESET_REQUIRED = 0x2000000,
    PGNI_MATRIX_SPLITTER = 0x800000, PGNI_MATRIX_COMBINER = 0x1000000;
struct Def { const char* id; unsigned flags; bool mono_audio, stereo_audio; };
struct Plugin {
    enum { POST_WEIGHT = 2000 };
    Def def;
    int active = 1, resident = 1, position = 0, pp = 1, lane = 0, source = 0, matrix_resident = 0;
    Def* get_pdef() { return &def; }
    int get_on_off() const { return active; }
    int get_scene_resident() const { return resident; }
    int get_position() const { return position; }
    int get_effect_post_pre() const { return pp; }
    int get_matrix_lane() const { return lane; }
    int get_matrix_source() const { return source; }
    int get_matrix_resident() const { return matrix_resident; }
    static void apply_matrix_state(void*, double) {}
    static double read_matrix_state(void*) { return 0; }
};
}
using gx_engine::Plugin;
struct Selector {
    Plugin plugin{{"unused", gx_engine::PGNI_DYN_POSITION, false, false}};
    vector<Plugin*> children;
    vector<Plugin*> get_modules() const { return children; }
};
struct Plugins : map<string, Plugin*> {
    bool validate_matrix(const map<string, int>&, string&) { return true; }
    Plugin* lookup_plugin(const string& id) { auto i = find(id); return i == end() ? nullptr : i->second; }
    Plugin* find_plugin(const string& id) { return lookup_plugin(id); }
};
struct Chain {
    list<Plugin*> modules;
    const list<Plugin*>& resident_modules() const { return modules; }
};
struct Engine {
    Selector crybaby, crybaby_second, wah, wah_second, tonestack, ampstack, poweramps;
    Plugins pluginlist;
    Chain mono_chain, stereo_chain;
};
list<Plugin*> compile(Engine& engine, const map<string, double>& values) {
#include "matrix_bank_selectors_under_test.h"
    return mono;
}
namespace gx_engine {
struct IntParameter {
    int value = 0;
    void set_snapshot_value(int v) { value = v; }
    int get_value() const { return value; }
};
struct BoolParameter {
    bool value = false;
    void set_snapshot_value(bool v) { value = v; }
    bool get_value() const { return value; }
};
struct FloatParameter {
    float value = 0;
    float* snapshot_pointer() { return &value; }
    float get_value() const { return value; }
    vector<int> signal_changed() const { return {1}; }
};
struct Parameter {
    char type = 'i';
    string group;
    bool snapshot_safe = false, midi_blocked = true;
    IntParameter integer;
    BoolParameter boolean;
    FloatParameter real;
    Parameter() = default;
    Parameter(char t, const string& g): type(t), group(g) {}
    bool isInt() const { return type == 'i'; }
    bool isBool() const { return type == 'b'; }
    bool isFloat() const { return type == 'f'; }
    const string& group_id() const { return group; }
    IntParameter& getInt() { return integer; }
    BoolParameter& getBool() { return boolean; }
    FloatParameter& getFloat() { return real; }
    void set_midi_blocked(bool v) { midi_blocked = v; }
};
}
struct Parameters : map<string, gx_engine::Parameter> {
    bool hasId(const string& id) const { return count(id); }
};
#include "matrix_bank_structural_under_test.h"
vector<gx_engine::SnapshotControl> compile_values(Engine& engine, Parameters& parameters,
                                                 const map<string, double>& values) {
    set<Plugin*> stereo_resident;
    vector<gx_engine::SnapshotControl> controls;
    vector<gx_engine::SnapshotGuard> guards;
#include "matrix_bank_values_under_test.h"
    return controls;
}
int main() {
    Engine engine;
    auto& proxy = engine.tonestack.plugin;
    proxy.def.id = "amp.tonestack";
    proxy.position = 5;
    Plugin selected{{"tone.active", PGN_ALTERNATIVE, true, false}};
    Plugin alternative{{"tone.parked", PGN_ALTERNATIVE, true, false}};
    selected.position = alternative.position = 5;
    alternative.active = 0;
    engine.tonestack.children = {&selected, &alternative};
    Plugin pedal{{"pedal", gx_engine::PGNI_DYN_POSITION, true, false}};
    pedal.position = 10;
    engine.pluginlist = Plugins{{{"amp.tonestack", &proxy}, {"tone.active", &selected},
                                {"tone.parked", &alternative}, {"pedal", &pedal}}};
    engine.mono_chain.modules = {&selected, &alternative, &pedal};
    assert(compile(engine, {}) == engine.mono_chain.modules);
    auto moved = compile(engine, {{"amp.tonestack.position", 20}});
    assert((moved == list<Plugin*>{&pedal, &selected, &alternative}));
    // Compilation must not move or toggle the audible graph.
    assert(proxy.position == 5 && selected.position == 5 && alternative.active == 0);
    auto guarded = [&](const map<string, double>& values) {
        try { compile(engine, values); assert(false); }
        catch (const RpcError& error) { assert(error.code == -32004); }
    };
    guarded({{"amp.tonestack.on_off", 0}});
    guarded({{"amp.tonestack.matrix_lane", 1}});
    engine.mono_chain.modules.remove(&selected);
    guarded({}); // A proxy cannot hide a missing selected DSP child.
    engine.mono_chain.modules.push_front(&selected);
    engine.mono_chain.modules.remove(&pedal);
    guarded({}); // Real missing processors must still be rejected.
    engine.mono_chain.modules.push_back(&pedal);
    pedal.def.stereo_audio = true;
    pedal.matrix_resident = 1;
    guarded({{"pedal.matrix_resident", 0}}); // Ownership transfer still guarded.

    Plugin fixed{{"amp.bass_boost", 0, true, false}};
    Plugin boundary{{"matrix_combiner", gx_engine::PGNI_DYN_POSITION, true, false}};
    engine.pluginlist["amp.bass_boost"] = &fixed;
    engine.pluginlist["matrix_combiner"] = &boundary;
    pedal.def.stereo_audio = false;
    engine.mono_chain.modules.push_back(&boundary);
    guarded({{"matrix_combiner.on_off", 0}});
    boundary.active = 0;
    guarded({{"matrix_combiner.on_off", 1}});
    compile(engine, {{"matrix_combiner.on_off", 0}});
    Parameters parameters;
    parameters["amp.bass_boost.position"] = {'i', "amp.bass_boost"};
    parameters["ui.matrix_combiner"] = {'b', "ui"};
    parameters["matrix_combiner.on_off"] = {'b', "matrix_combiner"};
    parameters["amp.tonestack.select"] = {'i', "amp.tonestack"};
    parameters["amp.tonestack.select"].integer.value = 2;
    auto controls = compile_values(engine, parameters,
        {{"amp.bass_boost.position", 40}, {"ui.matrix_combiner", 1}, {"amp.tonestack.select", 2}});
    assert(parameters["amp.bass_boost.position"].integer.value == 0);
    assert(!parameters["ui.matrix_combiner"].boolean.value);
    for (const auto& control : controls) {
        assert(control.stream == 0);
        control.apply(control.target, control.value);
    }
    assert(parameters["amp.bass_boost.position"].integer.value == 40);
    assert(parameters["ui.matrix_combiner"].boolean.value);
    assert(!parameters["matrix_combiner.on_off"].midi_blocked);
    for (const auto& control : compile_values(engine, parameters, {{"ui.matrix_combiner", 0}}))
        control.apply(control.target, control.value);
    assert(!parameters["ui.matrix_combiner"].boolean.value);
    assert(parameters["matrix_combiner.on_off"].midi_blocked);
    try { compile_values(engine, parameters, {{"amp.tonestack.select", 3}}); assert(false); }
    catch (const RpcError& error) { assert(error.code == -32004); }
}
