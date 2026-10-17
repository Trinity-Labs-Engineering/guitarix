// Execute the production bank prepare/promote RPC handlers. DSP compilation is
// counted here; separate native fixtures exercise the actual arrays and audio.
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "gx_snapshot.h"
#include "matrix_bank_capacity_under_test.h"
using namespace std;
struct RpcError : runtime_error {
    int code; string message;
    RpcError(int c, string m): runtime_error(m), code(c), message(m) {}
};
struct Value {
    string text; double number = 0;
    string getString() const { return text; }
    double getFloat() const { return number; }
};
struct Writer {
    map<string, bool> flags;
    vector<string> handles;
    void begin_object() { flags.clear(); handles.clear(); }
    void end_object() {}
    template<class T> void write_kv(const char*, const T&) {}
    void write_bool_kv(const char* key, bool value) { flags[key] = value; }
    void write_key(const char*) {}
    void begin_array() {}
    void end_array() {}
    void write(const string& handle) { handles.push_back(handle); }
};
struct Chain {
    const int* selected = nullptr;
    bool owns_bank(const int* data) const { return selected == data; }
};
struct Nam { void materialize_matrix_model() {} };
struct PreparedMatrixScene {
    string handle, matrix_id, structure;
    unsigned generation = 1, runtime_epoch = 1, jack_generation = 1;
};
struct PreparedMatrixCutover {
    shared_ptr<PreparedMatrixScene> recipe;
    gx_engine::PreparedSnapshot snapshot;
};
struct PreparedMatrixBank {
    shared_ptr<PreparedMatrixScene> recipe;
    vector<int> mono, stereo;
    map<string, shared_ptr<PreparedMatrixCutover>> cutovers;
    Chain* mono_chain;
    Chain* stereo_chain;
    bool acknowledged = false;
    string acknowledged_handle;
    uint32_t frame = 0;
};
// Run the production scheduler synchronously here. The chain/audio fixture
// separately checks real cutover timing, allocation and processor lifecycle.
struct Scheduler : gx_engine::SnapshotScheduler {
    bool schedule(gx_engine::PreparedSnapshot& snapshot, uint32_t target, uint32_t now,
                  unsigned block, unsigned fade) {
        if (!gx_engine::SnapshotScheduler::schedule(snapshot, target, now, block, fade)) return false;
        for (unsigned frame = target; frame < target + fade; frame += block) {
            begin(0, frame); finish(0, frame, block);
            begin(1, frame); finish(1, frame, block);
        }
        return true;
    }
};
struct Engine {
    Scheduler snapshots;
    Chain mono_chain, stereo_chain;
    Nam neural_amp, neural_amp_second, sneural_amp;
    vector<shared_ptr<void>> matrix_bank_owners;
    bool get_rack_changed() { return false; }
    bool scene_commit_ready() { return true; }
    unsigned get_samplerate() { return 48000; }
    unsigned get_buffersize() { return 32; }
    bool drained = true;
    struct Wait { bool done; bool finished() { return done; } };
    Wait wait_scene_control_cycle(bool, bool) { return {drained}; }
};
struct Jack {
    Engine engine;
    uint32_t now = 0;
    uint32_t get_time_is() { return now; }
    Engine& get_engine() { return engine; }
    struct Status { bool ok = true, active = true; unsigned generation = 1; };
    Status get_jack_client_activity_status() { return {}; }
};
struct Service {
    Jack jack;
    unsigned matrix_generation = 1;
    string active_matrix;
    map<string, shared_ptr<PreparedMatrixScene>> matrices;
    map<string, shared_ptr<PreparedMatrixBank>> matrix_banks;
    shared_ptr<PreparedMatrixBank> current_bank, next_bank;
#include "matrix_bank_lookup_under_test.h"
};
struct Connection {
    Service serv;
    unsigned images = 0, snapshots = 0;
    double last_parse_dispatch_ms = 0;
    shared_ptr<PreparedMatrixBank> compile_matrix_bank(shared_ptr<PreparedMatrixScene> recipe,
                                                     shared_ptr<PreparedMatrixBank> bank = nullptr) {
        if (!bank) {
            ++images;
            bank = make_shared<PreparedMatrixBank>(); bank->recipe = recipe;
            bank->mono = {1}; bank->stereo = {1};
            bank->mono_chain = &serv.jack.engine.mono_chain;
            bank->stereo_chain = &serv.jack.engine.stereo_chain;
        }
        ++snapshots;
        auto cutover = make_shared<PreparedMatrixCutover>(); cutover->recipe = recipe;
        cutover->snapshot.generation = serv.jack.engine.snapshots.generation();
        cutover->snapshot.matrix_handle = recipe->handle;
        cutover->snapshot.pool_generation = serv.matrix_generation;
        cutover->snapshot.controls = {
            {bank.get(), [](void* p, double) {
                auto* b = static_cast<PreparedMatrixBank*>(p); b->mono_chain->selected = b->mono.data();
            }, 0, 0},
            {bank.get(), [](void* p, double) {
                auto* b = static_cast<PreparedMatrixBank*>(p); b->stereo_chain->selected = b->stereo.data();
            }, 0, 1},
        };
        bank->cutovers[recipe->handle] = cutover;
        return bank;
    }
    void call(int method, vector<Value*> params, Writer& jw) {
        struct Method { int m_id; } entry{method};
        const auto* mn = &entry;
        switch(method) {
#define FUNCTION(name) case name:
#include "matrix_bank_rpc_under_test.h"
#undef FUNCTION
        }
    }
    enum { matrix_prepare_next, matrix_promote, snapshot_activate, RPCM_matrix_activate_next };
    void prepare(const string& handle, Writer& writer) {
        Value target{handle}; call(matrix_prepare_next, {&target}, writer);
    }
    void promote(const string& handle, uint32_t frame, Writer& writer) {
        Value target{handle}, when{"", double(frame)};
        call(matrix_promote, {&target, &when}, writer);
    }
    void activate(const string& handle, uint32_t frame, Writer& writer) {
        Value target{handle}, when{"", double(frame)};
        serv.jack.now = frame - 320;
        call(RPCM_matrix_activate_next, {&target, &when}, writer);
    }
};
template<class F> void rejects(F f, int code) {
    try { f(); assert(false); } catch (const RpcError& e) { assert(e.code == code); }
}
int main() {
    Connection c; Writer w;
    for (const string handle : {"A1", "A2", "B1", "B2", "C1", "C2", "D1", "D2", "E1", "E2", "F1"}) {
        auto r = make_shared<PreparedMatrixScene>();
        r->handle = handle; r->matrix_id = handle.substr(0,1); r->structure = r->matrix_id;
        c.serv.matrices[handle] = r;
    }
    c.serv.active_matrix = "A1";
    c.prepare("A2", w); assert(w.flags["sameMatrix"]); assert(c.images == 0);
    c.prepare("B1", w); assert(c.images == 2 && c.snapshots == 4 && w.handles.size() == 4);
    c.serv.jack.engine.drained = false;
    rejects([&] { c.prepare("C1", w); }, -32000);
    assert(c.images == 2);
    c.serv.jack.engine.drained = true;
    for (const string target : {"C1", "D1", "E1"}) c.prepare(target, w);
    assert(c.images == 5 && c.snapshots == 10 && w.handles.size() == 10);
    assert(c.serv.jack.engine.matrix_bank_owners.size() == 5);
    const auto banks = c.serv.matrix_banks;
    c.prepare("F1", w);
    assert(w.flags["requiresGuard"] && !w.flags["pointerSwitchReady"]);
    assert(c.images == 5 && c.serv.matrix_banks == banks);
    assert(!c.serv.bank_for_handle("F1") && !c.serv.bank_for_handle("missing"));
    const auto generation = c.serv.jack.engine.snapshots.generation();
    rejects([&] { c.promote("B1", 32, w); }, -32003);
    rejects([&] { c.activate("missing", 320, w); }, -32000);
    rejects([&] { c.activate("F1", 320, w); }, -32000);
    rejects([&] { c.activate("A1", 320, w); }, -32003);
    // All 20 directed edges, both snapshots, repeated without a prepare RPC.
    // The target deliberately differs from the most recently prepared bank.
    for (unsigned i = 0; i < 1000; ++i) {
        const unsigned previous = c.serv.active_matrix[0] - 'A';
        const unsigned target_index = (previous + 1 + (i / 5) % 4) % 5;
        const string target = string(1, 'A' + target_index) + (i % 2 ? "1" : "2");
        auto outgoing = c.serv.current_bank;
        const uint32_t frame = 320 + i * 32;
        c.activate(target, frame, w);
        assert(w.flags["matrixBankActivated"]);
        assert(c.serv.jack.engine.mono_chain.selected == banks.at(target.substr(0, 1))->mono.data());
        assert(c.serv.jack.engine.stereo_chain.selected == banks.at(target.substr(0, 1))->stereo.data());
        rejects([&] { c.prepare(target, w); }, -32000);
        rejects([&] { c.activate("A1", frame + 32, w); }, -32000);
        rejects([&] { c.promote(target, frame + 1, w); }, -32003);
        c.promote(target, frame, w);
        assert(c.serv.active_matrix == target);
        assert(c.serv.current_bank == banks.at(target.substr(0, 1)));
        assert(c.serv.next_bank == outgoing);
        assert(!c.serv.next_bank->acknowledged);
        assert(c.images == 5 && c.snapshots == 10);
        assert(c.serv.jack.engine.snapshots.generation() == generation);
    }
    // A newly prepared snapshot reuses its existing image even though its
    // recipe handle was not in readyHandles when the five images were built.
    const string alternate = c.serv.current_bank->recipe->matrix_id == "A" ? "B1" : "A1";
    auto added = make_shared<PreparedMatrixScene>(*c.serv.matrices.at(alternate));
    added->handle = "extra";
    c.serv.matrices[added->handle] = added;
    c.prepare("extra", w);
    assert(w.flags["bankReused"] && c.images == 5 && c.snapshots == 11);
    // Released recipes cannot be activated through retained image controls.
    c.serv.matrices.erase("extra");
    rejects([&] { c.activate("extra", 640, w); }, -32000);
    // Invalidating controls while an image remains selected must not free it.
    auto selected = c.serv.current_bank;
    c.serv.jack.engine.snapshots.invalidate();
    c.prepare(alternate, w);
    assert(w.flags["requiresGuard"] && !w.flags["pointerSwitchReady"]);
    assert(c.serv.current_bank == selected && c.images == 5);
    // Guarded reconciliation retires selected pointers, then a new generation
    // can reclaim the stale five images and admit a previously uncached matrix.
    c.serv.jack.engine.mono_chain.selected = nullptr;
    c.serv.jack.engine.stereo_chain.selected = nullptr;
    c.prepare("F1", w);
    assert(w.flags["pointerSwitchReady"] && c.images == 7);
    assert(c.serv.matrix_banks.size() == 2 && c.serv.jack.engine.matrix_bank_owners.size() == 2);
    assert(w.handles.size() == 3);
    c.serv.jack.engine.snapshots.invalidate_runtime();
    rejects([&] { c.prepare("F1", w); }, -32003);
    cout << "1000 promotions; five images, ten snapshots\n";
}
