// Execute the production bank prepare/promote RPC handlers. DSP compilation is
// counted here; separate native fixtures exercise the actual arrays and audio.
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "gx_snapshot.h"
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
    bool acknowledged = false;
    string acknowledged_handle;
    uint32_t frame = 0;
};
struct Engine {
    gx_engine::SnapshotScheduler snapshots;
    Chain mono_chain, stereo_chain;
    Nam neural_amp, neural_amp_second, sneural_amp;
    vector<shared_ptr<void>> matrix_bank_owners;
    bool get_rack_changed() { return false; }
    bool scene_commit_ready() { return true; }
    struct Wait { bool finished() { return true; } };
    Wait wait_scene_control_cycle(bool, bool) { return {}; }
};
struct Jack {
    Engine engine;
    Engine& get_engine() { return engine; }
    struct Status { bool ok = true, active = true; unsigned generation = 1; };
    Status get_jack_client_activity_status() { return {}; }
};
struct Service {
    Jack jack;
    unsigned matrix_generation = 1;
    string active_matrix;
    map<string, shared_ptr<PreparedMatrixScene>> matrices;
    shared_ptr<PreparedMatrixBank> current_bank, next_bank;
};
struct Connection {
    Service serv;
    unsigned images = 0, snapshots = 0;
    shared_ptr<PreparedMatrixBank> compile_matrix_bank(shared_ptr<PreparedMatrixScene> recipe,
                                                     shared_ptr<PreparedMatrixBank> bank = nullptr) {
        if (!bank) {
            ++images;
            bank = make_shared<PreparedMatrixBank>(); bank->recipe = recipe;
            bank->mono = {1}; bank->stereo = {1};
        }
        ++snapshots;
        auto cutover = make_shared<PreparedMatrixCutover>(); cutover->recipe = recipe;
        cutover->snapshot.generation = serv.jack.engine.snapshots.generation();
        bank->cutovers[recipe->handle] = cutover;
        return bank;
    }
    void call(int method, vector<Value*> params, Writer& jw) {
        switch(method) {
#define FUNCTION(name) case name:
#include "matrix_bank_rpc_under_test.h"
#undef FUNCTION
        }
    }
    enum { matrix_prepare_next, matrix_promote };
    void prepare(const string& handle, Writer& writer) {
        Value target{handle}; call(matrix_prepare_next, {&target}, writer);
    }
    void promote(const string& handle, uint32_t frame, Writer& writer) {
        Value target{handle}, when{"", double(frame)};
        call(matrix_promote, {&target, &when}, writer);
    }
    void ack(const string& handle, uint32_t frame) {
        auto bank = serv.next_bank;
        assert(bank->cutovers.count(handle));
        serv.jack.engine.mono_chain.selected = bank->mono.data();
        serv.jack.engine.stereo_chain.selected = bank->stereo.data();
        bank->acknowledged = true; bank->acknowledged_handle = handle; bank->frame = frame;
    }
};
template<class F> void rejects(F f, int code) {
    try { f(); assert(false); } catch (const RpcError& e) { assert(e.code == code); }
}
int main() {
    Connection c; Writer w;
    for (const string handle : {"A1", "A2", "A3", "B1", "B2"}) {
        auto r = make_shared<PreparedMatrixScene>();
        r->handle = handle; r->matrix_id = handle.substr(0,1); r->structure = r->matrix_id;
        c.serv.matrices[handle] = r;
    }
    c.serv.active_matrix = "A1";
    c.prepare("A2", w); assert(w.flags["sameMatrix"]); assert(c.images == 0);
    c.prepare("B1", w); assert(c.images == 2 && c.snapshots == 5 && w.handles.size() == 5);
    auto a = c.serv.current_bank, b = c.serv.next_bank;
    const auto generation = c.serv.jack.engine.snapshots.generation();
    rejects([&] { c.promote("B1", 32, w); }, -32003);
    for (unsigned i = 0; i < 1000; ++i) {
        const string target = i % 2 ? (i % 3 ? "A2" : "A3") : (i % 4 ? "B1" : "B2");
        c.prepare(target, w); assert(w.flags["bankReused"]);
        c.ack(target, i * 32);
        rejects([&] { c.prepare(target, w); }, -32000);
        rejects([&] { c.promote(target, i * 32 + 1, w); }, -32003);
        c.promote(target, i * 32, w);
        assert(c.serv.active_matrix == target);
        assert(c.serv.current_bank == (i % 2 ? a : b));
        assert(c.serv.next_bank == (i % 2 ? b : a));
        assert(!c.serv.next_bank->acknowledged);
        assert(c.images == 2 && c.snapshots == 5);
        assert(c.serv.jack.engine.snapshots.generation() == generation);
    }
    // Invalidating controls while an image remains selected must not free it.
    auto selected = c.serv.current_bank;
    c.serv.jack.engine.snapshots.invalidate();
    c.prepare("B1", w);
    assert(w.flags["requiresGuard"] && !w.flags["pointerSwitchReady"]);
    assert(c.serv.current_bank == selected && c.images == 2);
    c.serv.jack.engine.snapshots.invalidate_runtime();
    rejects([&] { c.prepare("B1", w); }, -32003);
    cout << "1000 A/B promotions; two images, five snapshots\n";
}
