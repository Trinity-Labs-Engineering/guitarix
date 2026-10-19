#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Glib { using ustring = std::string; }
namespace boost { struct mutex : std::mutex { using scoped_lock = std::lock_guard<mutex>; }; }
namespace nam { struct DSP { int identity; explicit DSP(int id) : identity(id) {} }; }
constexpr std::size_t kNamModelCacheEntries = 8;
constexpr const char* kNamNone = "None";
int model_loads = 0;
std::unique_ptr<nam::DSP> load_nam_model(const std::string& path, int rate,
    int* sample_rate, const char*, float, const char*) {
    if (path == "missing") return nullptr;
    ++model_loads;
    *sample_rate = rate;
    return std::unique_ptr<nam::DSP>(new nam::DSP(model_loads));
}
struct PreparedNamModelDescriptor { char slot; Glib::ustring filename; float size; };
struct PreparedNamModelResult {
    int requested, active_hits, cache_hits, loaded, capacity, host_sample_rate;
    bool rapid_switch_ready;
};
class NeuralAmp {
public:
    struct CachedNamModel {
        Glib::ustring filename;
        float size;
        int sample_rate, host_sample_rate;
        std::unique_ptr<nam::DSP> model;
    };
    nam::DSP* model = nullptr;
    Glib::ustring current_file;
    float current_model_size = 0.49f;
    int fSampleRate = 48000, mSampleRate = 48000;
    bool is_inited = true;
    std::vector<CachedNamModel> model_cache;
    std::vector<PreparedNamModelDescriptor> prepared_models;
    ~NeuralAmp() { delete model; }
    std::unique_ptr<nam::DSP> take_cached_model(const Glib::ustring&, float, int*);
    void cache_current_model();
    bool prepared_key_matches(const Glib::ustring&, float) const;
    PreparedNamModelResult prepare_song_models(const std::vector<PreparedNamModelDescriptor>&);
    int select(const std::string& name) {
        auto next = take_cached_model(name, current_model_size, &mSampleRate);
        assert(next);
        cache_current_model();
        model = next.release(); current_file = name;
        return model->identity;
    }
};

struct Convproc { enum { ST_STOP, ST_PROC }; };
struct GxJConvSettings {
    std::string path;
    float gain = 1.0f;
    bool operator==(const GxJConvSettings& other) const { return path == other.path && gain == other.gain; }
    std::string getFullIRPath() const { return path; }
    bool getGainCor() const { return true; }
    float getGain() const { return gain; }
    unsigned int getDelay() const { return 0; }
    unsigned int getOffset() const { return 0; }
    unsigned int getLength() const { return 0; }
    std::vector<int> getGainline() const { return {}; }
};
int ir_loads = 0;
struct GxConvolver {
    unsigned int rate = 48000, buffer = 32;
    int id = 0, status = Convproc::ST_STOP;
    bool ready = false;
    void set_samplerate(unsigned int value) { rate = value; }
    void set_buffersize(unsigned int value) { buffer = value; }
    unsigned int get_samplerate() const { return rate; }
    unsigned int get_buffersize() const { return buffer; }
    void set_sync(bool) {}
    bool configure(const std::string& path, float, float, unsigned int,
        unsigned int, unsigned int, unsigned int, int, int, const std::vector<int>&) {
        if (path == "missing") return false;
        id = ++ir_loads; return true;
    }
    bool is_runnable() const { return ready; }
    void stop_process() { ready = false; status = Convproc::ST_STOP; }
    bool checkstate() const { return true; }
    int state() const { return status; }
    bool start(int, int) { ready = true; status = Convproc::ST_PROC; return true; }
};
struct Engine { void get_sched_priority(int& policy, int& priority) { policy = priority = 0; } };
struct Plugin { void set_on_off(bool) {} };
void gx_print_warning(const char*, const char*) {}
#define _(s) s
class ConvolverAdapter {
public:
    std::unique_ptr<GxConvolver> conv{new GxConvolver()};
    std::atomic<GxConvolver*> audio_convolver{nullptr};
    struct CachedIR {
        GxJConvSettings settings;
        unsigned int samplerate, buffersize;
        std::unique_ptr<GxConvolver> convolver;
    };
    std::vector<CachedIR> ir_cache;
    std::vector<GxJConvSettings> prepared_irs;
    bool convolver_sync = false;
    boost::mutex activate_mutex;
    Engine engine;
    Plugin plugin;
    int barriers = 0;
    std::function<void()> sync = [&] {
        // Retirement must unpublish before crossing the callback barrier.
        assert(audio_convolver.load() == nullptr);
        ++barriers;
    };
    GxJConvSettings jcset, configured_jcset;
    unsigned int configured_samplerate = 0, configured_buffersize = 0;
    bool has_configured_ir = false;
    bool configuration_matches();
    void suspend_convolver();
    bool ir_is_prepared(const GxJConvSettings&) const;
    std::unique_ptr<GxConvolver> configure_ir(const GxJConvSettings&);
    PreparedNamModelResult prepare_song_irs(const std::vector<GxJConvSettings>&);
    bool configure_current_ir();
    bool conv_prepare();
    bool conv_start();
};

#include "prepared_resources_under_test.h"

int main() {
    NeuralAmp pedal;
    auto prepared = pedal.prepare_song_models({{'N', "drive", .49f}, {'N', "fuzz", .49f}});
    assert(prepared.rapid_switch_ready && prepared.loaded == 2 && pedal.model == nullptr);
    const int drive = pedal.select("drive"), fuzz = pedal.select("fuzz");
    for (int i = 0; i < 10; ++i) {
        assert(pedal.select("drive") == drive);
        assert(pedal.select("fuzz") == fuzz);
    }
    assert(model_loads == 2);
    prepared = pedal.prepare_song_models({{'N', "drive", .49f}, {'N', "fuzz", .49f}});
    assert(prepared.active_hits == 1 && prepared.cache_hits == 1 && prepared.loaded == 0);
    assert(!pedal.prepare_song_models({{'N', "missing", .49f}}).rapid_switch_ready);
    pedal.cache_current_model();
    pedal.fSampleRate = 96000;
    prepared = pedal.prepare_song_models({{'N', "drive", .49f}});
    assert(prepared.loaded == 1 && prepared.rapid_switch_ready);

    NeuralAmp full;
    std::vector<PreparedNamModelDescriptor> models;
    for (int i = 0; i < 9; ++i) models.push_back({'N', std::to_string(i), .49f});
    prepared = full.prepare_song_models(models);
    assert(!prepared.rapid_switch_ready && prepared.loaded == 8 && full.model_cache.size() == 8);
    // An unrelated outgoing model cannot displace any promised song resource.
    full.model = new nam::DSP(999); full.current_file = "adhoc";
    full.cache_current_model();
    assert(full.model_cache.size() == 8 && full.select("0") != 999);
    models.erase(models.begin() + 1, models.end());
    full.prepare_song_models(models);
    assert(full.prepare_song_models({{'N', "replacement", .49f}}).rapid_switch_ready);

    ConvolverAdapter ir;
    prepared = ir.prepare_song_irs({{"clean"}, {"lead"}});
    assert(prepared.rapid_switch_ready && prepared.loaded == 2);
    assert(ir.audio_convolver.load() == nullptr && !ir.conv->is_runnable());
    ir.jcset = {"clean"}; assert(ir.conv_start());
    const int clean = ir.conv->id;
    const auto* audible = ir.audio_convolver.load();
    prepared = ir.prepare_song_irs({{"clean"}, {"lead"}, {"third"}});
    assert(prepared.active_hits == 1 && prepared.cache_hits == 1 && prepared.loaded == 1);
    assert(ir.audio_convolver.load() == audible && ir.conv->is_runnable());
    const int loaded = ir_loads;
    for (int i = 0; i < 10; ++i) {
        ir.jcset = {"lead"}; ir.suspend_convolver(); assert(ir.conv_start());
        assert(ir.audio_convolver.load() == ir.conv.get());
        ir.jcset = {"clean"}; ir.suspend_convolver(); assert(ir.conv_start());
        assert(ir.conv->id == clean);
    }
    assert(ir_loads == loaded && ir.barriers == 20);
    ir.suspend_convolver(); assert(ir.conv_prepare()); assert(ir.conv_start());
    assert(ir_loads == loaded); // bypass -> enable reuses configuration
    ir.jcset.gain = 0.5f; ir.suspend_convolver(); assert(ir.conv_start());
    assert(ir_loads == loaded + 1); // settings are part of the key
    ir.suspend_convolver(); ir.conv->set_samplerate(96000); assert(ir.conv_start());
    assert(ir_loads == loaded + 2);
    ir.suspend_convolver(); ir.conv->set_buffersize(64); assert(ir.conv_start());
    assert(ir_loads == loaded + 3);
    assert(!ir.prepare_song_irs({{"missing"}}).rapid_switch_ready);

    ConvolverAdapter limited;
    std::vector<GxJConvSettings> settings;
    for (int i = 0; i < 9; ++i) settings.push_back({std::to_string(i)});
    prepared = limited.prepare_song_irs(settings);
    assert(!prepared.rapid_switch_ready && prepared.loaded == 8 && limited.ir_cache.size() == 8);
    prepared = limited.prepare_song_irs({{"replacement"}});
    assert(prepared.rapid_switch_ready && limited.ir_cache.size() == 8);
    std::cout << "prepared-resources-ok\n";
}
