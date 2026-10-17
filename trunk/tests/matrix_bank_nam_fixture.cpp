// Production NAM bank selection/ownership and audio code, with an instrumented
// model backend. No JACK, UI, application build, or asynchronous fake worker.
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>
#include "gx_nam_dc.h"
static unsigned allocations = 0, constructions = 0, destructions = 0;
static bool fail_allocation = false;
void* operator new(std::size_t n) { ++allocations; if (fail_allocation) throw std::bad_alloc(); if (void* p = std::malloc(n)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
namespace Glib { using ustring = std::string; }
namespace gx_system { template<class T> T atomic_get(T value) { return value; } template<class T> void atomic_set(T* p, T v) { *p = v; } }
namespace nam {
struct DSP {
    float gain, bias;
    unsigned frames = 0;
    explicit DSP(float g): gain(g), bias(g / 200) { ++constructions; }
    ~DSP() { ++destructions; }
    void process(float** in, float** out, int count) {
        frames += count;
        for (int i = 0; i < count; ++i) out[0][i] = in[0][i] * gain + bias;
    }
};
}
static void update_nam_level_calibration(nam::DSP*, float* in, float* out, float* loudness,
                                         const char*, const std::string&, const char*) {
    *in = *out = *loudness = 0;
}
using gx_engine::remove_nam_dc;
#define AVOIDDENORMALS()
#include "matrix_bank_nam_process_under_test.h"
struct Resampler {
    int max_out_count(int n) { return n; }
    int up(int n, float*, float*) { assert(false); return n; }
    void down(float*, float*) { assert(false); }
};
struct Ramp {
    enum { OFF, DOWN, DEAD, UP } mode = OFF;
    void rampDown(int, float*) { assert(false); }
    void rampUp(int, float*) { assert(false); }
};
#define always_inline
class NeuralAmp {
public:
    struct MatrixModel { NeuralAmp* owner; nam::DSP* model; Glib::ustring filename; float size, input_trim, output_trim, loudness; float dc_offset = 0; };
    struct CachedNamModel { Glib::ustring filename; float size; int sample_rate, host_sample_rate; std::unique_ptr<nam::DSP> model; float dc_offset = 0; };
    std::atomic<const MatrixModel*> matrix_model{nullptr};
    nam::DSP* model = new nam::DSP(0.5f);
    float model_dc_offset = gx_engine::measure_nam_dc(model);
    std::vector<CachedNamModel> model_cache;
    Glib::ustring current_file = "current", load_file;
    float fVslider0 = 0, fVslider1 = 0, fVslider2 = .49f, current_model_size = .49f;
    float nam_input_trim_db = 0, nam_output_trim_db = 0, loudness = 0;
    int mSampleRate = 48000, fSampleRate = 48000, ready = 1, need_resample = 0, scene_smoother_snap_pending = 0;
    double fRec0[2]{1, 1}, fRec1[2]{1, 1};
    std::vector<float> scratch;
    Resampler smp;
    Ramp ramp;
    ~NeuralAmp() { delete model; }
    void publish_runtime_status(nam::DSP*, const std::string&) {}
    bool compile_matrix_model(const Glib::ustring&, float, MatrixModel&);
    static void apply_matrix_model(void*, double);
    void materialize_matrix_model();
    void preset_scene_smoother_targets();
    void compute(int, float*, float*);
};
#include "matrix_bank_nam_under_test.h"
int main() {
    {
        NeuralAmp amp;
        const std::string names[] = {"current", "next", "third", "fourth", "fifth"};
        const float gains[] = {.5f, -.5f, .25f, -.25f, .75f};
        for (unsigned i = 1; i < 5; ++i)
            amp.model_cache.push_back({names[i], .49f, 48000, 48000, std::unique_ptr<nam::DSP>(new nam::DSP(gains[i]))});
        amp.model_cache.push_back({"different-rate", .49f, 44100, 48000, std::unique_ptr<nam::DSP>(new nam::DSP(1))});
        for (auto& entry : amp.model_cache) entry.dc_offset = gx_engine::measure_nam_dc(entry.model.get());
        NeuralAmp::MatrixModel banks[5];
        assert(!amp.compile_matrix_model("different-rate", .49f, banks[1]));
        assert(!amp.compile_matrix_model("absent", .49f, banks[1]));
        for (unsigned i = 0; i < 5; ++i)
            assert(amp.compile_matrix_model(names[i], .49f, banks[i]));
        float input[32], output[32];
        unsigned seed = 7;
        for (unsigned i = 0; i < 1000; ++i) {
            seed = seed * 1664525u + 1013904223u;
            const unsigned target = (seed >> 8) % 5;
            auto& bank = banks[target];
            const auto heap = allocations, created = constructions, destroyed = destructions;
            NeuralAmp::apply_matrix_model(&bank, 0);
            for (unsigned j = 0; j < 32; ++j) input[j] = std::sin((i * 32 + j) * .02f);
            amp.compute(32, input, output);
            for (unsigned j = 0; j < 32; ++j) assert(std::fabs(output[j] - input[j] * gains[target]) < 1e-6f);
            assert(allocations == heap && constructions == created && destructions == destroyed);
            // Promotion transfers ownership without resetting either DSP.
            const unsigned frames = bank.model->frames;
            amp.materialize_matrix_model();
            assert(amp.model == bank.model && amp.model->frames == frames);
            assert(amp.model_dc_offset == amp.model->bias);
            for (const auto& entry : amp.model_cache) assert(entry.dc_offset == entry.model->bias);
            assert(amp.model_cache.size() == 5 && constructions == created && destructions == destroyed);
        }
        NeuralAmp::MatrixModel pending;
        assert(amp.compile_matrix_model("next", .49f, pending));
        NeuralAmp::apply_matrix_model(&pending, 0);
        // Pool release/song change must preserve the audible borrowed model.
        amp.materialize_matrix_model();
        amp.model_cache.clear();
        amp.compute(32, input, output);
        assert(amp.model->gain == -.5f);
    }
    {
        NeuralAmp amp;
        std::string filename(128, 'n');
        amp.model_cache.push_back({filename, .49f, 48000, 48000, std::unique_ptr<nam::DSP>(new nam::DSP(-.5f))});
        NeuralAmp::MatrixModel selection;
        assert(amp.compile_matrix_model(filename, .49f, selection));
        NeuralAmp::apply_matrix_model(&selection, 0);
        auto* original = amp.model;
        fail_allocation = true;
        bool rejected = false;
        try { amp.materialize_matrix_model(); } catch (const std::bad_alloc&) { rejected = true; }
        fail_allocation = false;
        assert(rejected && amp.model == original);
        assert(amp.matrix_model.load() == &selection && amp.model_cache[0].model.get() == selection.model);
        amp.materialize_matrix_model();
        assert(amp.model == selection.model && amp.model_cache[0].model.get() == original);
    }
    assert(constructions == destructions);
    std::cout << "matrix-bank-nam-ok: 1000 transitions\n";
}
