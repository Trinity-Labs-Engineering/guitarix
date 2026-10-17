// Exercise preparation with the production GxConvolver, resampler and zita DSP.
// No JACK server or Guitarix application is built or started by this fixture.
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sched.h>
#include <string>
#include <thread>
#include <vector>
#include <boost/format.hpp>
#include <glib.h>
using namespace std;
#define __rt_func
#define gx_print_error(...) ((void)0)
#define gx_print_warning(...) ((void)0)
#define gx_print_info(...) ((void)0)
#include "gx_resampler.h"
#include "gx_convolver.h"
#include "convolver_under_test.cpp"
#include "resampler_under_test.cpp"
using gx_engine::GxConvolver;
namespace boost { struct mutex { struct scoped_lock { explicit scoped_lock(mutex&) {} }; }; }
struct GxJConvSettings {
    string file;
    Gainline envelope = [] { Gainline g; g.push_back({0, 0}); g.push_back({-1, 0}); return g; }();
    bool operator==(const GxJConvSettings& other) const { return file == other.file && envelope == other.envelope; }
    bool getGainCor() const { return false; }
    float getGain() const { return 1; }
    string getFullIRPath() const { return file; }
    unsigned getDelay() const { return 0; }
    unsigned getOffset() const { return 0; }
    unsigned getLength() const { return 0; }
    Gainline getGainline() const { return envelope; }
};
struct Engine {
    unsigned rate = 48000, block = 32;
    unsigned get_samplerate() const { return rate; }
    unsigned get_buffersize() const { return block; }
};
struct ConvolverAdapter {
    unique_ptr<GxConvolver> conv{new GxConvolver};
    struct PreparedIR { GxJConvSettings settings; unique_ptr<GxConvolver> processor; };
    vector<PreparedIR> prepared_irs;
    boost::mutex activate_mutex;
    Engine engine;
    GxJConvSettings jcset, configured_jcset;
    unsigned configured_samplerate = 0, configured_buffersize = 0;
    bool has_configured_ir = false;
    bool configure_current_ir();
    bool matrix_ir_ready(const GxJConvSettings&);
    bool prepare_matrix_ir(const GxJConvSettings&);
};
#include "ir_pool_under_test.h"
static size_t thread_count() {
    return distance(filesystem::directory_iterator("/proc/self/task"), filesystem::directory_iterator());
}
int main(int argc, char** argv) {
    assert(argc == 2);
    for (float gain : {0.f, -12.f, 6.f}) {
        gain_points a{0, gain}, b{0, gain}, different{0, gain + 1};
        assert(a == b);
        assert(a != different);
    }
    const string path = string(argv[1]) + "/impulse.wav";
    const auto baseline_threads = thread_count();
    for (unsigned block : {32u, 64u, 128u}) for (int rate : {44100, 48000}) for (int channels : {1, 2}) {
        SF_INFO info{};
        info.samplerate = rate; info.channels = channels; info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        SNDFILE* file = sf_open(path.c_str(), SFM_WRITE, &info);
        assert(file);
        vector<float> ir(65536 * channels, 0.0f);
        for (int ch = 0; ch < channels; ++ch) ir[ch] = 1.0f;
        assert(sf_writef_float(file, ir.data(), 65536) == 65536);
        sf_close(file);
        ConvolverAdapter adapter;
        adapter.engine.block = block;
        adapter.conv->set_samplerate(48000);
        adapter.conv->set_buffersize(block);
        const auto* untouched = adapter.conv.get();
        for (int i = 0; i < 10; ++i) {
            assert(adapter.prepare_matrix_ir({path}));
            assert(adapter.matrix_ir_ready({path}));
            assert(adapter.conv.get() == untouched);
            assert(adapter.prepared_irs.size() == 1);
            assert(adapter.prepared_irs[0].processor->state() == Convproc::ST_STOP);
            assert(!adapter.prepared_irs[0].processor->is_runnable());
            assert(thread_count() == baseline_threads);
        }
        adapter.jcset = {path};
        assert(adapter.configure_current_ir());
        assert(adapter.prepared_irs.empty());
        // Playback of the prepared processor must work. Synchronize all
        // partition workers in this offline test before stopping them.
        adapter.conv->set_sync(true);
        assert(adapter.conv->start(SCHED_OTHER, 0));
        vector<float> input(block, 0), left(block), right(block);
        input[0] = 1;
        float peak = 0;
        for (unsigned samples = 0; samples < 32768; samples += block) {
            assert(adapter.conv->compute(block, input.data(), input.data(), left.data(), right.data()));
            for (unsigned i = 0; i < block; ++i) { assert(isfinite(left[i]) && isfinite(right[i])); peak = max(peak, abs(left[i])); }
            input[0] = 0;
        }
        assert(peak > 0.1f);
        adapter.conv->set_not_runnable();
        adapter.conv->stop_process();
        const auto deadline = chrono::steady_clock::now() + chrono::seconds(2);
        while (!adapter.conv->checkstate() && chrono::steady_clock::now() < deadline)
            this_thread::sleep_for(chrono::milliseconds(1));
        assert(adapter.conv->state() == Convproc::ST_STOP);
        assert(!adapter.prepare_matrix_ir({path + ".missing"}));
    }
    cout << "matrix-ir-real-ok\n";
}
