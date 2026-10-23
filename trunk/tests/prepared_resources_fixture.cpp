#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cerrno>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Glib {
using ustring = std::string;
std::string build_filename(const std::string& directory, const std::string& file) {
    return directory.empty() ? file : directory + "/" + file;
}
}
namespace boost { struct mutex : std::mutex { using scoped_lock = std::lock_guard<mutex>; }; }
namespace nam { struct DSP { int identity; explicit DSP(int id) : identity(id) {} }; }
#include "nam_cache_config_under_test.h"
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

class NeuralAmpMulti {
public:
    struct CachedNamModel {
        char slot;
        Glib::ustring filename;
        float size;
        int sample_rate, host_sample_rate;
        Glib::ustring pin_generation;
        std::unique_ptr<nam::DSP> model;
    };
    nam::DSP* modela = nullptr;
    nam::DSP* modelb = nullptr;
    Glib::ustring current_afile, current_bfile, prepared_generation;
    float current_model_sizea = 0.49f, current_model_sizeb = 0.49f;
    int fSampleRate = 48000;
    bool is_inited = true;
    std::vector<CachedNamModel> model_cache;
    std::vector<PreparedNamModelDescriptor> prepared_models;
    ~NeuralAmpMulti() { delete modela; delete modelb; }
    std::unique_ptr<nam::DSP> take_cached_model(char, const Glib::ustring&, float, int*);
    void cache_model(nam::DSP*&, const Glib::ustring&, float, int, char);
    bool prepared_key_matches(char, const Glib::ustring&, float) const;
    PreparedNamModelResult prepare_song_models(const Glib::ustring&,
        const std::vector<PreparedNamModelDescriptor>&);
    int select(char slot, const std::string& name) {
        int rate = 0;
        auto next = take_cached_model(slot, name, .49f, &rate);
        assert(next);
        auto& active = slot == 'A' ? modela : modelb;
        auto& current = slot == 'A' ? current_afile : current_bfile;
        cache_model(active, current, .49f, rate, slot);
        active = next.release(); current = name;
        return active->identity;
    }
};

struct Convproc { enum { ST_STOP, ST_PROC }; };
// Use the real settings type and equality, including gainline points. Simplified
// cache keys hid a production bug where zero-dB points were unequal to themselves.
#define G_BEGIN_DECLS
#define G_END_DECLS
#include <gxwmm/gainline.h>
#undef G_BEGIN_DECLS
#undef G_END_DECLS
using std::string;
using guint = unsigned int;
namespace gx_system { class JsonParser; class JsonWriter; }
template <typename T> class ParameterV;
#include "ir_settings_under_test.h"
GxJConvSettings ir_settings(const std::string& path) {
    GxJConvSettings settings;
    settings.setIRFile(path);
    settings.setGain(1.0f);
    settings.setGainCor(true);
    Gainline envelope;
    envelope.push_back({0, 0.0});
    envelope.push_back({-1, 0.0});
    settings.setGainline(envelope);
    return settings;
}
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
        unsigned int, unsigned int, unsigned int, int, int, const Gainline&) {
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
    for (std::size_t i = 0; i <= kNamModelCacheEntries; ++i)
        models.push_back({'N', std::to_string(i), .49f});
    prepared = full.prepare_song_models(models);
    assert(!prepared.rapid_switch_ready && prepared.loaded == (int)kNamModelCacheEntries &&
           full.model_cache.size() == kNamModelCacheEntries);
    // An unrelated outgoing model cannot displace any promised song resource.
    full.model = new nam::DSP(999); full.current_file = "adhoc";
    full.cache_current_model();
    assert(full.model_cache.size() == kNamModelCacheEntries && full.select("0") != 999);
    models.erase(models.begin() + 1, models.end());
    full.prepare_song_models(models);
    assert(full.prepare_song_models({{'N', "replacement", .49f}}).rapid_switch_ready);

    // Five dual-NAM presets plus next song's default must prepare from an empty
    // amp wrapper and survive repeated complete bank changes without loading.
    NeuralAmpMulti amp;
    models.clear();
    for (int i = 0; i < 6; ++i)
        for (char slot : {'A', 'B'}) models.push_back({slot, std::to_string(i), .49f});
    prepared = amp.prepare_song_models("song-and-next", models);
    assert(prepared.rapid_switch_ready && prepared.loaded == 12);
    assert(prepared.capacity == (int)kNamModelCacheEntries);
    const int bank_loads = model_loads;
    for (int pass = 0; pass < 4; ++pass) {
        for (int i = 0; i < 6; ++i) {
            const int a = amp.select('A', std::to_string(i));
            const int b = amp.select('B', std::to_string(i));
            assert(a != b); // each channel has independent recurrent state
        }
    }
    assert(model_loads == bank_loads);
    prepared = amp.prepare_song_models("song-and-next", models);
    assert(prepared.rapid_switch_ready && prepared.active_hits == 2 && prepared.cache_hits == 10);
    models.push_back({'A', "previous-song-default", .49f});
    models.push_back({'B', "previous-song-default", .49f});
    prepared = amp.prepare_song_models("both-song-neighbours", models);
    assert(prepared.rapid_switch_ready); // 12 cached graphs plus two already active
    NeuralAmpMulti from_empty;
    prepared = from_empty.prepare_song_models("both-song-neighbours", models);
    assert(prepared.rapid_switch_ready == (kNamModelCacheEntries >= 14));
    assert(prepared.loaded == (int)std::min<std::size_t>(14, kNamModelCacheEntries));

    auto flat = ir_settings("flat");
    assert(flat == flat);
    Gainline envelope;
    envelope.push_back({0, -6.0});
    envelope.push_back({-1, 0.0});
    auto attenuated = flat;
    attenuated.setGainline(envelope);
    assert(attenuated == attenuated && !(attenuated == flat));
    auto adjusted = attenuated;
    envelope[0].g = -3.0;
    adjusted.setGainline(envelope);
    assert(!(adjusted == attenuated));
    flat.setGain(-1.0f);
    assert(flat == flat);
    flat.setGain(0.0f);
    assert(flat == flat);

    // Match Houston's six-scene payload, including its zero-dB envelope.
    ConvolverAdapter song;
    std::vector<GxJConvSettings> six_configs;
    for (int i = 0; i < 6; ++i) six_configs.push_back(ir_settings(std::to_string(i)));
    prepared = song.prepare_song_irs(six_configs);
    assert(prepared.loaded == 6 && prepared.rapid_switch_ready);
    const int song_loads = ir_loads;
    for (int pass = 0; pass < 3; ++pass) {
        prepared = song.prepare_song_irs(six_configs);
        assert(prepared.requested == 6 && prepared.cache_hits == 6 && prepared.loaded == 0);
        assert(prepared.rapid_switch_ready && ir_loads == song_loads);
    }
    six_configs.push_back(six_configs.front());
    prepared = song.prepare_song_irs(six_configs);
    assert(prepared.requested == 6 && prepared.cache_hits == 6 && prepared.loaded == 0);

    ConvolverAdapter ir;
    prepared = ir.prepare_song_irs({ir_settings("clean"), ir_settings("lead")});
    assert(prepared.rapid_switch_ready && prepared.loaded == 2);
    assert(ir.audio_convolver.load() == nullptr && !ir.conv->is_runnable());
    ir.jcset = ir_settings("clean"); assert(ir.conv_start());
    const int clean = ir.conv->id;
    const auto* audible = ir.audio_convolver.load();
    prepared = ir.prepare_song_irs({ir_settings("clean"), ir_settings("lead"), ir_settings("third")});
    assert(prepared.active_hits == 1 && prepared.cache_hits == 1 && prepared.loaded == 1);
    assert(ir.audio_convolver.load() == audible && ir.conv->is_runnable());
    const int loaded = ir_loads;
    for (int i = 0; i < 10; ++i) {
        ir.jcset = ir_settings("lead"); ir.suspend_convolver(); assert(ir.conv_start());
        assert(ir.audio_convolver.load() == ir.conv.get());
        ir.jcset = ir_settings("clean"); ir.suspend_convolver(); assert(ir.conv_start());
        assert(ir.conv->id == clean);
    }
    assert(ir_loads == loaded && ir.barriers == 20);
    ir.suspend_convolver(); assert(ir.conv_prepare()); assert(ir.conv_start());
    assert(ir_loads == loaded); // bypass -> enable reuses configuration
    ir.jcset.setGain(0.5f); ir.suspend_convolver(); assert(ir.conv_start());
    assert(ir_loads == loaded + 1); // settings are part of the key
    ir.suspend_convolver(); ir.conv->set_samplerate(96000); assert(ir.conv_start());
    assert(ir_loads == loaded + 2);
    ir.suspend_convolver(); ir.conv->set_buffersize(64); assert(ir.conv_start());
    assert(ir_loads == loaded + 3);
    assert(!ir.prepare_song_irs({ir_settings("missing")}).rapid_switch_ready);

    ConvolverAdapter limited;
    std::vector<GxJConvSettings> settings;
    for (int i = 0; i < 9; ++i) settings.push_back(ir_settings(std::to_string(i)));
    prepared = limited.prepare_song_irs(settings);
    assert(!prepared.rapid_switch_ready && prepared.loaded == 8 && limited.ir_cache.size() == 8);
    prepared = limited.prepare_song_irs({ir_settings("replacement")});
    assert(prepared.rapid_switch_ready && limited.ir_cache.size() == 8);
    std::cout << "prepared-resources-ok\n";
}
