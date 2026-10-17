// Production mNAM preparation, cache, selection and scene batching with a
// counted DSP loader: no JACK server or real model files are required.
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Glib { using ustring = std::string; }
namespace gx_system {
template <class T> T atomic_get(T value) { return value; }
template <class T> void atomic_set(T* value, T replacement) { *value = replacement; }
}
namespace nam { struct DSP { float size; }; }
static int constructions = 0, resizes = 0;
static constexpr std::size_t kNamModelCacheEntries = 8;
static const char* kNamNone = "None";
static void gx_print_info(const char*, const std::string&) {}
static bool nam_model_is_slimmable(nam::DSP*) { return true; }
static bool set_nam_slimmable_size(nam::DSP* model, float size, int, int,
                                  const char*, const std::string&, const char*, float* dc_offset) {
    ++resizes;
    model->size = size;
    *dc_offset = size / 100;
    return true;
}
static std::unique_ptr<nam::DSP> load_nam_model(const std::string&, int rate,
    int* model_rate, const char*, float size, const char*, float* dc_offset) {
    ++constructions;
    *model_rate = rate;
    *dc_offset = size / 100;
    return std::unique_ptr<nam::DSP>(new nam::DSP{size});
}
static bool get_selected_nam_file(float index, const std::string& directory,
    const std::vector<std::string>& names, std::string* result, const char*) {
    if (index < 1 || index > names.size()) return false;
    *result = directory + "/" + names[static_cast<int>(index) - 1];
    return true;
}
static int max_nam_model_block_frames(int, int) { return 64; }
static int setup_nam_resampler(int, int, int) { return 0; }
static void update_nam_level_calibration(nam::DSP*, float*, float*, double*,
                                         const char*, const std::string&, const char*) {}
struct PreparedNamModelDescriptor { char slot; std::string filename; float size; };
struct PreparedNamModelResult {
    int requested, active_hits, cache_hits, loaded, capacity, host_sample_rate;
    bool rapid_switch_ready;
};
struct CachedNamModel {
    char slot;
    std::string filename;
    float size;
    int sample_rate, host_sample_rate;
    std::string pin_generation;
    std::unique_ptr<nam::DSP> model;
    float dc_offset = 0;
};
struct Ramp {
    enum Mode { OFF, DOWN, UP };
    Mode mode = OFF;
    void startRampDown() { mode = DOWN; }
};
class NeuralAmpMulti {
public:
    bool is_inited = true;
    int fSampleRate = 48000, maSampleRate = 48000, mbSampleRate = 48000;
    int ready = 0, barriers = 0;
    nam::DSP* modela = nullptr;
    nam::DSP* modelb = nullptr;
    float model_dc_offseta = 0, model_dc_offsetb = 0;
    float fVslider3 = 1, fVslider4 = 1, current_model_sizea = 1, current_model_sizeb = 1;
    float afilelist = 1, bfilelist = 1;
    std::string load_apath = "/models", load_bpath = "/models";
    std::string load_afile, load_bfile, current_afile, current_bfile;
    std::vector<std::string> nam_afile_names{"shared.nam", "other.nam"};
    std::vector<std::string> nam_bfile_names{"shared.nam", "other.nam"};
    std::vector<float> scratch_modela, scratch_modelb;
    int smpa = 0, smpb = 0, need_aresample = 0, need_bresample = 0;
    double loudnessa = 0, loudnessb = 0;
    float nam_input_trim_dba = 0, nam_output_trim_dba = 0, nam_input_trim_dbb = 0, nam_output_trim_dbb = 0;
    Ramp rampA, rampB;
    bool muted_scene_parameter_batch = false, scene_smoother_preset = false;
    bool scene_parameter_batch = false, scene_model_a_pending = false, scene_model_b_pending = false;
    std::string prepared_generation;
    std::vector<PreparedNamModelDescriptor> prepared_models;
    std::vector<CachedNamModel> model_cache;
    const Glib::ustring* matrix_selection = nullptr;
    std::vector<PreparedNamModelDescriptor> matrix_model_inventory() const;
    bool matrix_model_ready(char, const Glib::ustring&, float) const;
    void select_matrix_model(char, const Glib::ustring&, float);
    ~NeuralAmpMulti() { delete modela; delete modelb; }
    void sync() { ++barriers; }
    void clear_state_f() {}
    void preset_scene_smoother_targets() {}
    void publish_runtime_status(bool, const nam::DSP*, const std::string&) {}
    void begin_scene_parameter_batch(bool);
    bool finish_scene_parameter_batch();
    void load_nam_afile_impl();
    void load_nam_bfile_impl();
    void load_nam_afile();
    void load_nam_bfile();
    void set_nam_asize();
    void set_nam_bsize();
    std::unique_ptr<nam::DSP> take_cached_model(char, const std::string&, float, int*, float*);
    bool prepared_key_matches(char, const std::string&, float) const;
    void cache_model(nam::DSP*&, const std::string&, float, int, char);
    PreparedNamModelResult prepare_song_models(const std::string&, const std::vector<PreparedNamModelDescriptor>&);
};
#include "nam_pool_under_test.h"

int main() {
    NeuralAmpMulti amp;
    std::vector<PreparedNamModelDescriptor> plan;
    for (char slot : {'A', 'B'}) {
        for (float size : {0.49f, 1.0f}) plan.push_back({slot, "/models/shared.nam", size});
        plan.push_back({slot, "/models/other.nam", 0.49f});
    }
    auto prepared = amp.prepare_song_models("song", plan);
    assert(prepared.rapid_switch_ready && prepared.loaded == 6 && constructions == 6);
    // Includes a same-file profile change and a simultaneous file/profile
    // change. Intermediate other.nam/standard was deliberately not prepared.
    for (int repeat = 0; repeat < 100; ++repeat) {
        for (int scene = 0; scene < 3; ++scene) {
            const float size = scene == 0 ? 1.0f : 0.49f;
            const int selector = scene == 2 ? 2 : 1;
            int before = amp.barriers;
            amp.begin_scene_parameter_batch(true);
            amp.fVslider3 = amp.fVslider4 = size;
            amp.set_nam_asize();
            amp.set_nam_bsize();
            // Directory callbacks can run with the previous file selector.
            amp.load_nam_afile_impl();
            amp.load_nam_bfile_impl();
            amp.afilelist = amp.bfilelist = selector;
            amp.load_nam_afile_impl();
            amp.load_nam_bfile_impl();
            assert(amp.barriers == before);
            assert(amp.finish_scene_parameter_batch());
            assert(amp.barriers == before + 2);
            assert(amp.modela && amp.modelb && amp.modela != amp.modelb);
            assert(amp.modela->size == size && amp.modelb->size == size);
            assert(amp.model_dc_offseta == size / 100 && amp.model_dc_offsetb == size / 100);
            for (const auto& entry : amp.model_cache) assert(entry.dc_offset == entry.size / 100);
            assert(amp.current_afile == (selector == 1 ? "/models/shared.nam" : "/models/other.nam"));
            assert(amp.current_bfile == amp.current_afile);
            assert(constructions == 6 && resizes == 0);
            assert(!amp.finish_scene_parameter_batch()); // Cleanup cannot apply twice.
        }
    }
    int before = amp.barriers;
    amp.begin_scene_parameter_batch(true);
    amp.load_nam_afile_impl();
    amp.load_nam_bfile_impl();
    assert(!amp.finish_scene_parameter_batch());
    assert(amp.barriers == before); // An unchanged selection has no ownership wait.
    auto reused = amp.prepare_song_models("same-pool", plan);
    assert(reused.rapid_switch_ready && reused.loaded == 0 && constructions == 6);
    // Prepared matrices select absolute identities, independent of the UI's
    // directory/selector table. Displacing a model parks it without warming.
    for (int repeat = 0; repeat < 1000; ++repeat) {
        const float size = repeat % 2 ? 1.0f : 0.49f;
        amp.begin_scene_parameter_batch(true);
        amp.select_matrix_model('A', "/models/shared.nam", size);
        amp.select_matrix_model('B', "/models/other.nam", 0.49f);
        amp.finish_scene_parameter_batch();
        assert(amp.current_afile == "/models/shared.nam" && amp.modela->size == size);
        assert(amp.current_bfile == "/models/other.nam");
        assert(amp.matrix_model_inventory().size() == 6);
        assert(constructions == 6 && resizes == 0);
    }
    auto* active = amp.modela;
    bool rejected = false;
    try { amp.select_matrix_model('A', "/models/missing.nam", 0.49f); }
    catch (const std::runtime_error&) { rejected = true; }
    assert(rejected && amp.modela == active && constructions == 6);
    // Interactive size controls retain their existing behavior.
    amp.select_matrix_model('A', "/models/shared.nam", 0.49f);
    amp.fVslider3 = 1.0f;
    amp.set_nam_asize();
    assert(resizes == 1 && amp.modela->size == 1.0f);
    std::cout << "matrix-nam-pool-ok\n";
}
