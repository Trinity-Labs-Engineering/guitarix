from pathlib import Path
import os
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class CabinetSceneReadyTests(unittest.TestCase):
    def test_pending_update_owns_callback_state_and_propagates_failure(self):
        source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        body = source.split("bool FixedBaseConvolver::settle_muted_scene(bool pending) {", 1)[1].split("\nint FixedBaseConvolver::activate(", 1)[0]
        fixture = r"""
#include <atomic>
#include <cassert>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <thread>
namespace boost { struct mutex : std::mutex { using scoped_lock = std::lock_guard<mutex>; }; }
class FixedBaseConvolver {
public:
    boost::mutex activate_mutex;
    bool activated = true;
    struct Plugin { bool enabled = true; bool get_on_off() { return enabled; } } plugin;
    std::atomic<bool> scene_update_suspended{false}, updating{false}, done{false};
    std::atomic<unsigned> callbacks{0}, accesses{0};
    unsigned syncs = 0, starts = 0;
    int result = 1;
    bool pending_ir = true;
    void sync() {
        ++syncs;
        const auto initial = callbacks.load();
        while (callbacks.load() == initial) std::this_thread::yield();
    }
    bool start() {
        assert(scene_update_suspended.load());
        ++starts;
        updating.store(true);
        auto initial = accesses.load();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        assert(accesses.load() == initial);
        updating.store(false);
        if (result < 0) throw std::runtime_error("IR allocation failed");
        if (result) pending_ir = false;
        return result;
    }
    bool settle_muted_scene(bool pending);
};
bool FixedBaseConvolver::settle_muted_scene(bool pending) {
""" + body + r"""
int main() {
    FixedBaseConvolver cabinet;
    std::thread audio([&] {
        while (!cabinet.done.load()) {
            if (!cabinet.scene_update_suspended.load(std::memory_order_acquire)) {
                ++cabinet.accesses;
                std::this_thread::yield();
            }
            ++cabinet.callbacks;
        }
    });
    for (int i = 0; i < 40; ++i) {
        cabinet.pending_ir = true;
        assert(cabinet.settle_muted_scene(true));
        assert(!cabinet.pending_ir); // no delayed timer reload remains
        assert(!cabinet.scene_update_suspended.load());
    }
    auto syncs = cabinet.syncs, starts = cabinet.starts;
    assert(cabinet.settle_muted_scene(false)); // unchanged cabinet is free
    cabinet.activated = false;
    assert(cabinet.settle_muted_scene(true)); // first activation owns loading
    cabinet.activated = true;
    cabinet.plugin.enabled = false;
    assert(cabinet.settle_muted_scene(true)); // bypassed cabinets need no wait
    assert(cabinet.syncs == syncs && cabinet.starts == starts);
    cabinet.plugin.enabled = true;
    cabinet.pending_ir = true;
    cabinet.result = 0;
    assert(!cabinet.settle_muted_scene(true));
    assert(cabinet.pending_ir && !cabinet.scene_update_suspended.load());
    cabinet.result = -1;
    try { cabinet.settle_muted_scene(true); assert(false); }
    catch (const std::runtime_error&) {}
    assert(!cabinet.scene_update_suspended.load());
    cabinet.result = 1;
    assert(cabinet.settle_muted_scene(true)); // retry must remain possible
    cabinet.done = true;
    audio.join();
}
"""
        with tempfile.TemporaryDirectory(prefix="gx-cabinet-scene-") as directory:
            cpp = Path(directory) / "fixture.cpp"
            binary = Path(directory) / "fixture"
            cpp.write_text(fixture)
            subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror", "-pthread", str(cpp), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=10)


    def test_cabinet_cache_is_published_only_after_successful_start(self):
        source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        for stereo in (False, True):
            name = "CabinetStereoConvolver" if stereo else "CabinetConvolver"
            begin = source.index("bool " + name + "::do_update() {")
            end = source.index("\nvoid " + name + "::check_update()", begin)
            methods = source[begin:end].replace(name, "Cabinet")
            fixture = r"""
#include <cassert>
struct CabDesc { int ir_count = 1, ir_sr = 96000; float ir_data[1]; };
struct Entry { CabDesc* data; };
Entry& getCabEntry(int index) {
    static CabDesc data[] = {{1,96000,{0}}, {1,96000,{1}}};
    static Entry entries[] = {{&data[0]}, {&data[1]}};
    return entries[index];
}
struct Cabinet {
    int cabinet = 0, current_cab = 0, syncs = 0, starts = 0;
    bool start_ok = true;
    struct Conv {
        bool ready = true, load_ok = true;
        int installed = 0, loads = 0;
        bool is_runnable() { return ready; }
        void set_not_runnable() { ready = false; }
        void stop_process() { ready = false; }
        bool checkstate() { return true; }
        bool configure(int, float* data, int) {
            ++loads;
            if (!load_ok) return false;
            installed = int(data[0]);
            return true;
        }
        bool configure_stereo(int n, float* data, int sr) { return configure(n,data,sr); }
        bool update(int n, float* data, int sr) { return configure(n,data,sr); }
        bool update_stereo(int n, float* data, int sr) { return configure(n,data,sr); }
    } conv;
    struct Filter { void clear_state_f() {} void init(unsigned) {} } impf;
    struct Resampler { void setup(unsigned,unsigned) {} } smp, smps;
    void sync() { ++syncs; }
    unsigned getSamplingFreq() { return 48000; }
    bool cabinet_changed() { return cabinet != current_cab; }
    void update_cabinet() { current_cab = cabinet; }
    bool conv_start() { ++starts; return conv.ready = start_ok; }
    bool do_update();
    bool start(bool force = false);
};
""" + methods + r"""
int main() {
    Cabinet cab;
    assert(cab.start());
    assert(cab.conv.loads == 0 && cab.starts == 0);
    cab.cabinet = 1;
    assert(cab.conv.installed == 0); // selector alone leaves the old IR live
    assert(cab.start());
    assert(cab.current_cab == 1 && cab.conv.installed == 1 && cab.conv.ready);
    cab.cabinet = 0;
    cab.conv.load_ok = false;
    assert(!cab.start());
    assert(cab.current_cab == -1);
    cab.conv.load_ok = true;
    cab.start_ok = false;
    assert(!cab.start());
    assert(cab.current_cab == -1 && !cab.conv.ready);
    cab.cabinet = 1; // returning to the old selector must rebuild after failure
    cab.start_ok = true;
    auto loads = cab.conv.loads;
    assert(cab.start());
    assert(cab.conv.loads == loads + 1 && cab.conv.installed == 1 && cab.conv.ready);
    assert(cab.current_cab == 1);
}
"""
            with self.subTest(stereo=stereo), tempfile.TemporaryDirectory(prefix="gx-cabinet-cache-") as directory:
                cpp = Path(directory) / "fixture.cpp"
                binary = Path(directory) / "fixture"
                cpp.write_text(fixture)
                subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)], check=True)
                subprocess.run([str(binary)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
