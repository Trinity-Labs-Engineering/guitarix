"""Actual zita workers must receive exact host blocks during scene replay."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]


class SceneConvolverReplayTests(unittest.TestCase):
    def test_synchronous_replay_matches_continuous_partitioned_convolution(self):
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest("C++ compiler unavailable")
        flags = subprocess.run(["pkg-config", "--cflags", "--libs", "fftw3f"], capture_output=True, text=True)
        if flags.returncode:
            self.skipTest("fftw3f development files unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_convolver.cpp").read_text()
        method = source.split("bool GxConvolver::compute_synchronous(", 1)[1].split(
            "bool __rt_func GxConvolver::compute(", 1)[0]
        fixture = r'''
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <vector>
#include "zita-convolver.h"
std::atomic<bool> allow_worker_start{true};
std::atomic<bool> reject_workers{false};
struct WorkerStart { void* (*entry)(void*); void* argument; };
void* delayed_worker(void* argument) {
    WorkerStart* start = static_cast<WorkerStart*>(argument);
    while (!allow_worker_start.load(std::memory_order_acquire)) usleep(100);
    auto entry = start->entry;
    auto payload = start->argument;
    delete start;
    return entry(payload);
}
extern "C" int __real_pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*);
extern "C" int __wrap_pthread_create(pthread_t* thread, const pthread_attr_t* attributes,
                                     void* (*entry)(void*), void* argument) {
    if (reject_workers.load()) return EAGAIN;
    auto start = new WorkerStart{entry, argument};
    const int result = __real_pthread_create(thread, attributes, delayed_worker, start);
    if (result) delete start;
    return result;
}
class GxConvolver : public Convproc {
public:
    unsigned int buffersize = 32;
    bool ready = true;
    bool is_runnable() const { return ready; }
    bool compute_synchronous(int, float*, float*);
};
'''
        fixture += "bool GxConvolver::compute_synchronous(" + method
        fixture += r'''
void verify_replay(bool no_workers) {
    GxConvolver replay, reference;
    std::vector<float> ir(24001, 0);
    ir[0] = .7f; ir[71] = -.2f; ir[8193] = .4f; ir[23999] = -.3f;
    for (GxConvolver* conv : {&replay, &reference}) {
        assert(conv->configure(1, 1, ir.size(), 32, 64, 8192) == 0);
        assert(conv->impdata_create(0, 0, 1, ir.data(), 0, ir.size()) == 0);
        reject_workers.store(no_workers);
        assert(conv->start_process(0, SCHED_OTHER) == 0);
    }
    float input[64], actual[64];
    std::fill(input, input + 64, 1.0f);
    assert(!replay.compute_synchronous(31, input, actual));
    assert(!replay.compute_synchronous(64, input, actual));
    replay.ready = false;
    assert(!replay.compute_synchronous(32, input, actual));
    replay.ready = true;
    // Ineligible calls above must not advance a partial/padded FFT quantum.
    // A strong tap at 500ms checks the slow worker partitions as well as the
    // initial response. Replay runs without wall-clock sleeps.
    const auto signal = [](long frame) {
        return frame < 0 ? 0.0 : .3 * std::sin(frame * .051) + .2 * std::sin(frame * .0031);
    };
    for (unsigned int offset = 0; offset < 50000; offset += 32) {
        for (unsigned int i = 0; i < 32; ++i)
            input[i] = float(signal(offset + i));
        std::memcpy(reference.inpdata(0), input, 32 * sizeof(float));
        assert(reference.process(true) == 0);
        assert(replay.compute_synchronous(32, input, actual));
        for (unsigned int i = 0; i < 32; ++i) {
            assert(std::fabs(actual[i] - reference.outdata(0)[i]) < 1e-6);
            // With quantum 32/minpart 64 the partitioned engine delays by 96
            // frames. Check sparse convolution analytically as well, so two
            // identically-wrong pipelines cannot satisfy this regression.
            const long frame = long(offset + i) - 96;
            const double expected = .7 * signal(frame) - .2 * signal(frame - 71)
                + .4 * signal(frame - 8193) - .3 * signal(frame - 23999);
            if (std::fabs(actual[i] - expected) >= 1e-5) {
                std::cerr << "FIR mismatch no_workers=" << no_workers << " frame=" << offset + i
                    << " actual=" << actual[i] << " expected=" << expected << "\n";
                assert(false);
            }
        }
    }
    // Hand ownership back without resetting/reconfiguring pending partitions.
    // The normal asynchronous path continues with scheduler margin for this
    // correctness test; hardware capture checks the real callback deadline.
    // Synchronous process can still have launched later jobs when it returns.
    for (unsigned int block = 0; block < 64; ++block) {
        usleep(10000);
        std::memcpy(reference.inpdata(0), input, 32 * sizeof(float));
        assert(reference.process(true) == 0);
        std::memcpy(replay.inpdata(0), input, 32 * sizeof(float));
        assert(replay.process(false) == 0);
        for (unsigned int i = 0; i < 32; ++i)
            assert(std::fabs(replay.outdata(0)[i] - reference.outdata(0)[i]) < 1e-6);
    }
    for (GxConvolver* conv : {&replay, &reference}) {
        assert(conv->stop_process() == 0);
        while (!conv->check_stop()) usleep(100);
        assert(conv->cleanup() == 0);
    }
}
int main() {
    // Stop before any successfully-created worker has reached its entry point.
    // A late worker must observe ST_TERM instead of overwriting it with ST_PROC.
    GxConvolver stopped_before_start;
    float impulse = 1;
    assert(stopped_before_start.configure(1, 1, 24001, 32, 64, 8192) == 0);
    assert(stopped_before_start.impdata_create(0, 0, 1, &impulse, 0, 1) == 0);
    allow_worker_start.store(false, std::memory_order_release);
    assert(stopped_before_start.start_process(0, SCHED_OTHER) == 0);
    assert(stopped_before_start.stop_process() == 0);
    allow_worker_start.store(true, std::memory_order_release);
    unsigned int attempts = 0;
    while (!stopped_before_start.check_stop() && attempts++ < 10000) usleep(100);
    assert(stopped_before_start.check_stop());
    assert(stopped_before_start.cleanup() == 0);
    // No startup sleeps or private state barrier: process(true) must be safe
    // immediately after the production start_process() publishes the engine.
    verify_replay(false);
    verify_replay(true); // Failed pthread_create keeps the synchronous fallback.
    std::cout << "scene-convolver-replay-ok\n";
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-scene-convolver-") as temporary:
            directory = Path(temporary)
            (directory / "convolver.cpp").write_text(fixture)
            executable = directory / "convolver"
            built = subprocess.run([
                compiler, "-std=c++17", "-O2", "-pthread",
                f"-I{TRUNK / 'src/zita-convolver'}", str(directory / "convolver.cpp"),
                str(TRUNK / "src/zita-convolver/zita-convolver.cc"),
                *flags.stdout.split(), "-Wl,--wrap=pthread_create", "-o", str(executable),
            ], capture_output=True, text=True, timeout=30)
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "scene-convolver-replay-ok")


if __name__ == "__main__":
    unittest.main()
