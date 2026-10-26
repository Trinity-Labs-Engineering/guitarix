"""Compare production dual-NAM control replay and normal callback math."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SceneNamSerialReplayTests(unittest.TestCase):
    def test_serial_replay_preserves_mix_state_and_live_dispatch(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_neural_plugins.cpp").read_text()
        start = source.index("void always_inline NeuralAmpMulti::compute(")
        end = source.index("// non rt callback", start)
        methods = source[start:end].replace("always_inline ", "")
        fixture = r'''
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>
namespace gx_system {
bool atomic_get(int value) { return value != 0; }
void atomic_set(int* value, int replacement) { *value = replacement; }
}
#include "gx_nam_block_adapter.h"
using gx_engine::kMaxNamHostBlockFrames;
using gx_engine::kNamModelBlockFrames;
struct PluginDef {};
struct Parallel {
    bool available = true, pending = false;
    int queries = 0, dispatches = 0, waits = 0;
    std::function<void()> job;
    bool getProcess() { ++queries; assert(!pending); return available; }
    void setProcessor(int processor) { assert(processor == 1); }
    void runProcess() { assert(!pending); pending = true; ++dispatches; }
    void processWait() { assert(pending); job(); pending = false; ++waits; }
};
struct NeuralAmpMulti : PluginDef {
    int ready = 1, scene_smoother_snap_pending = 1;
    unsigned int scene_discard_prefix_frames = 0;
    int model_token = 1;
    int* modela = &model_token;
    int* modelb = &model_token;
    double fVslider1 = -3.0, fVslider2 = 0.37, fVslider02 = 0;
    double fRec1[2] = {}, fRec2[2] = {};
    std::vector<float> scratcha = std::vector<float>(kMaxNamHostBlockFrames), scratchb = std::vector<float>(kMaxNamHostBlockFrames);
    int nframes = 0, a_calls = 0, b_calls = 0, delay_calls = 0, max_frames = 0;
    float* buf = nullptr;
    double a_state = 0, b_state = 0;
    std::vector<unsigned int> a_discards, b_discards;
    float delay_state = 0;
    Parallel worker;
    Parallel* pro = &worker;
    NeuralAmpMulti() { worker.job = [this] { processModelB(); }; }
    void preset_scene_smoother_targets() {
        fRec1[0] = fRec1[1] = std::pow(10.0, 0.05 * fVslider1);
        fRec2[0] = fRec2[1] = fVslider2;
    }
    void processDelay(int count, float* data) {
        ++delay_calls;
        for (int i = 0; i < count; ++i) {
            const float next = data[i]; data[i] = delay_state; delay_state = next;
        }
    }
    void processModelA(int count, float* data) {
        ++a_calls; a_discards.push_back(scene_discard_prefix_frames);
        max_frames = std::max(max_frames, count);
        for (int i = 0; i < count; ++i) {
            a_state = std::tanh(0.7 * data[i] + 0.13 * a_state);
            data[i] = float(a_state * 1.1);
        }
    }
    void processModelB() {
        ++b_calls; b_discards.push_back(scene_discard_prefix_frames);
        max_frames = std::max(max_frames, nframes);
        for (int i = 0; i < nframes; ++i) {
            b_state = std::tanh(-0.2 * buf[i] + 0.67 * b_state);
            buf[i] = float(b_state * 0.81);
        }
    }
    void compute(int, float*, float*, bool parallel = true, unsigned int discarded_prefix_frames = 0);
    static void compute_static(int, float*, float*, PluginDef*);
    void replay_scene_block(int, float*, unsigned int discarded_prefix_frames = 0, bool parallel = true);
};
#include "methods.h"
int main() {
    for (int models : {0, 1, 2, 3})
    for (double mix : {0.0, 0.37, 1.0})
    for (int alignment : {-1, 0, 1})
    for (bool worker_available : {false, true}) {
        NeuralAmpMulti serial, live;
        for (auto* model : {&serial, &live}) {
            if (!(models & 1)) model->modela = nullptr;
            if (!(models & 2)) model->modelb = nullptr;
            model->fVslider2 = mix; model->fVslider02 = alignment;
            model->worker.available = worker_available;
        }
        unsigned int cursor = 0;
        for (int count : {1, 32, 128, 256, 513, 8193}) {
            std::vector<float> a(count), b(count);
            for (int i = 0; i < count; ++i, ++cursor)
                a[i] = b[i] = float(.31 * std::sin(cursor * .071) + .07 * std::cos(cursor * .013));
            // Change an unsnapped live target mid-stream so equivalence also
            // covers evolving gain/mix smoothers rather than only endpoints.
            if (count == 513) {
                serial.fVslider1 = live.fVslider1 = 1.5;
                serial.fVslider2 = live.fVslider2 = 1.0 - mix;
            }
            serial.replay_scene_block(count, a.data(), 0, false);
            // Match the control replay block boundaries, as the engine does.
            // The normal wrapper remains the production dispatch entry.
            for (int offset = 0; offset < count; offset += kNamModelBlockFrames)
                NeuralAmpMulti::compute_static(std::min(kNamModelBlockFrames, count - offset),
                    b.data() + offset, b.data() + offset, &live);
            assert(std::memcmp(a.data(), b.data(), count * sizeof(float)) == 0);
            assert(serial.a_state == live.a_state && serial.b_state == live.b_state);
            assert(serial.delay_state == live.delay_state);
            assert(serial.fRec1[1] == live.fRec1[1] && serial.fRec2[1] == live.fRec2[1]);
        }
        assert(serial.worker.queries == 0 && serial.worker.dispatches == 0 && serial.worker.waits == 0);
        assert(serial.a_calls == live.a_calls && serial.b_calls == live.b_calls);
        assert(serial.delay_calls == live.delay_calls && serial.max_frames <= kNamModelBlockFrames);
        assert(live.max_frames <= kNamModelBlockFrames && live.worker.waits == live.worker.dispatches);
        if (models == 3) {
            assert(live.worker.queries > 0);
            assert((live.worker.dispatches > 0) == worker_available);
        } else assert(live.worker.dispatches == 0);
    }
    {
        NeuralAmpMulti cone;
        std::vector<float> data(1025, .25f);
        cone.replay_scene_block(data.size(), data.data(), 512);
        const std::vector<unsigned int> expected{512, 256, 0, 0, 0};
        assert(cone.a_discards == expected && cone.b_discards == expected);
        assert(cone.worker.dispatches == 5 && cone.worker.waits == 5);
        NeuralAmpMulti::compute_static(32, data.data(), data.data(), &cone);
        assert(cone.a_discards.back() == 0 && cone.b_discards.back() == 0);
    }
    std::cout << "scene-nam-serial-replay-ok\n";
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-serial-replay-") as temporary:
            root = Path(temporary)
            (root / "methods.h").write_text(methods)
            (root / "fixture.cpp").write_text(fixture)
            exe = root / "fixture"
            built = subprocess.run([compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                                    f"-I{root}", f"-I{TRUNK / 'src/headers'}", str(root / "fixture.cpp"), "-o", str(exe)],
                                   capture_output=True, text=True, timeout=30)
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "scene-nam-serial-replay-ok")


if __name__ == "__main__":
    unittest.main()
