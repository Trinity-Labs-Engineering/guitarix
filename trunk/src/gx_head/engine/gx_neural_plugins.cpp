/*
 * Copyright (C) 2024 Hermann Meyer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * --------------------------------------------------------------------------
 *
 *
 *    This is part of the Guitarix Audio Engine
 *
 *
 *
 * --------------------------------------------------------------------------
 */

#include "engine.h"
#include "gx_faust_support.h"
#include "calibration.h"
#include "container.h"
#include "lstm.h"
#include "model_config.h"
#include "slimmable.h"
#include "wavenet/model.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>

namespace gx_engine {

/****************************************************************
 ** NAM Core v0.5 adapter
 **
 ** Guitarix NAM modules are mono. NAM Core v0.5 processes arrays
 ** of channel pointers, so model loading validates the channel count
 ** and this adapter supplies the single Guitarix input/output channel.
 */

namespace {

constexpr int kMaxNamHostBlockFrames = 8192;
constexpr int kNamSelectorSlots = 128;
constexpr std::size_t kNamModelCacheEntries = 8;
constexpr float kDefaultNamSlimmableSize = 0.49f;
constexpr const char* kNamNone = "None";
constexpr double kDefaultNamReferenceLevelDbu = 12.0;
constexpr double kMaxNamAutomaticTrimDb = 60.0;

struct NamReferenceLevel {
    double dbu = kDefaultNamReferenceLevelDbu;
    const char* source = "default";
    bool invalid_override = false;
};

const NamReferenceLevel& nam_reference_level() {
    static const NamReferenceLevel reference = [] {
        NamReferenceLevel result;
        const char* raw = std::getenv("GUITARIX_NAM_REFERENCE_LEVEL_DBU");
        if (raw && *raw) {
            result.source = "GUITARIX_NAM_REFERENCE_LEVEL_DBU";
        } else {
            raw = std::getenv("ARTEMIS_NAM_REFERENCE_LEVEL_DBU");
            if (raw && *raw) {
                result.source = "ARTEMIS_NAM_REFERENCE_LEVEL_DBU";
            }
        }
        if (!raw || !*raw) {
            return result;
        }

        errno = 0;
        char* end = nullptr;
        const double parsed = std::strtod(raw, &end);
        if (errno != 0 || end == raw || *end != '\0' || !std::isfinite(parsed) ||
            parsed < -60.0 || parsed > 60.0) {
            result.dbu = kDefaultNamReferenceLevelDbu;
            result.invalid_override = true;
            return result;
        }
        result.dbu = parsed;
        return result;
    }();
    return reference;
}

void update_nam_level_calibration(nam::DSP* model,
                                  float* input_trim_db,
                                  float* output_trim_db,
                                  float* loudness,
                                  const char* module_name,
                                  const Glib::ustring& filename,
                                  const char* slot_name) {
    *input_trim_db = 0.0f;
    *output_trim_db = 0.0f;
    if (loudness) {
        *loudness = 0.0f;
    }
    if (!model) {
        return;
    }

    if (loudness && model->HasLoudness() && std::isfinite(model->GetLoudness())) {
        *loudness = static_cast<float>(model->GetLoudness());
    }

    const NamReferenceLevel& reference = nam_reference_level();
    const nam::LevelCalibration calibration =
        nam::GetLevelCalibration(*model, reference.dbu);
    const bool input_trim_rejected = calibration.input_trim_db.has_value() &&
        std::abs(calibration.input_trim_db.value()) > kMaxNamAutomaticTrimDb;
    const bool output_trim_rejected = calibration.output_trim_db.has_value() &&
        std::abs(calibration.output_trim_db.value()) > kMaxNamAutomaticTrimDb;
    if (calibration.input_trim_db.has_value() && !input_trim_rejected) {
        *input_trim_db = static_cast<float>(calibration.input_trim_db.value());
    }
    if (calibration.output_trim_db.has_value() && !output_trim_rejected) {
        *output_trim_db = static_cast<float>(calibration.output_trim_db.value());
    }

    std::ostringstream message;
    message << "NAM calibration " << slot_name << " for " << std::string(filename)
            << ": reference " << std::showpos << std::fixed << std::setprecision(2)
            << reference.dbu << " dBu";
    if (reference.invalid_override) {
        message << " (invalid " << reference.source << "; using default)";
    } else {
        message << " (" << reference.source << ")";
    }

    if (input_trim_rejected) {
        message << "; input " << calibration.model_input_level_dbu.value()
                << " dBu => trim outside +/-60.00 dB; ignored";
    } else if (calibration.input_trim_db.has_value()) {
        message << "; input " << calibration.model_input_level_dbu.value()
                << " dBu => " << calibration.input_trim_db.value() << " dB";
    } else {
        message << "; input metadata absent => +0.00 dB";
    }
    if (output_trim_rejected) {
        message << "; output " << calibration.model_output_level_dbu.value()
                << " dBu => trim outside +/-60.00 dB; ignored";
    } else if (calibration.output_trim_db.has_value()) {
        message << "; output " << calibration.model_output_level_dbu.value()
                << " dBu => " << calibration.output_trim_db.value() << " dB";
    } else {
        message << "; output metadata absent => +0.00 dB";
    }
    gx_print_info(module_name, message.str());
}

void ensure_nam_builtin_parsers_registered() {
    static const bool registered = [] {
        auto& registry = nam::ConfigParserRegistry::instance();
        if (!registry.has("Linear")) {
            registry.registerParser("Linear", nam::linear::create_config);
        }
        if (!registry.has("LSTM")) {
            registry.registerParser("LSTM", nam::lstm::create_config);
        }
        if (!registry.has("WaveNet")) {
            registry.registerParser("WaveNet", nam::wavenet::create_config);
        }
        if (!registry.has("SlimmableContainer")) {
            registry.registerParser("SlimmableContainer", nam::container::create_config);
        }
        return true;
    }();
    (void)registered;
}

// Allow for resampler rounding while preallocating the largest NAM block.
int max_nam_model_block_frames(int host_sample_rate, int model_sample_rate) {
    const int safe_host_sample_rate = host_sample_rate > 0 ? host_sample_rate : 1;
    const double ratio = static_cast<double>(model_sample_rate) /
        static_cast<double>(safe_host_sample_rate);
    return static_cast<int>(std::ceil(kMaxNamHostBlockFrames * ratio)) + 64;
}

bool initialize_nam_model(nam::DSP* model, int host_sample_rate,
                          int model_sample_rate, const char* module_name,
                          const Glib::ustring& filename) {
    if (model->NumInputChannels() != 1 || model->NumOutputChannels() != 1) {
        gx_print_info(module_name, "unsupported non-mono NAM model " +
                      std::string(filename));
        return false;
    }

    try {
        // Reset and prewarm outside the audio callback to avoid allocations there.
        model->ResetAndPrewarm(model_sample_rate,
                               max_nam_model_block_frames(host_sample_rate, model_sample_rate));
        return true;
    } catch (const std::exception& error) {
        gx_print_info(module_name, "fail to initialize " + std::string(filename) +
                      ": " + error.what());
        return false;
    }
}

bool nam_model_is_slimmable(nam::DSP* model) {
    return dynamic_cast<nam::SlimmableModel*>(model) != nullptr;
}

float clamp_nam_size(float size) {
    return std::max<float>(0.0f, std::min<float>(1.0f, size));
}

bool set_nam_slimmable_size(nam::DSP* model, float size, int host_sample_rate,
                            int model_sample_rate, const char* module_name,
                            const Glib::ustring& filename, const char* slot_name) {
    auto* slimmable = dynamic_cast<nam::SlimmableModel*>(model);
    if (!slimmable) {
        return false;
    }

    const float clamped_size = clamp_nam_size(size);
    try {
        slimmable->SetSlimmableSize(clamped_size);
        model->ResetAndPrewarm(model_sample_rate,
                               max_nam_model_block_frames(host_sample_rate, model_sample_rate));
        gx_print_info(module_name, std::string("set ") + slot_name + " size " +
                      std::to_string(clamped_size) + " for " + std::string(filename));
        return true;
    } catch (const std::exception& error) {
        gx_print_info(module_name, std::string("fail to set ") + slot_name +
                      " size for " + std::string(filename) + ": " + error.what());
    } catch (...) {
        gx_print_info(module_name, std::string("fail to set ") + slot_name +
                      " size for " + std::string(filename));
    }
    return false;
}

std::unique_ptr<nam::DSP> load_nam_model(const Glib::ustring& filename,
                                         int host_sample_rate,
                                         int* model_sample_rate,
                                         const char* module_name,
                                         float model_size,
                                         const char* slot_name) {
    try {
        ensure_nam_builtin_parsers_registered();
        std::unique_ptr<nam::DSP> next =
            nam::get_dsp(std::filesystem::path{std::string(filename)},
                         nam::PrewarmMode::SKIP);
        if (!next) {
            gx_print_info(module_name, "fail to load " + std::string(filename));
            return nullptr;
        }

        *model_sample_rate = static_cast<int>(next->GetExpectedSampleRate());
        if (*model_sample_rate <= 0) {
            *model_sample_rate = 48000;
        }
        if (auto* slimmable = dynamic_cast<nam::SlimmableModel*>(next.get())) {
            try {
                const float clamped_size = clamp_nam_size(model_size);
                slimmable->SetSlimmableSize(clamped_size);
                gx_print_info(module_name, std::string("selected ") + slot_name +
                              " size " + std::to_string(clamped_size) +
                              " for " + std::string(filename));
            } catch (const std::exception& error) {
                gx_print_info(module_name, std::string("fail to select ") + slot_name +
                              " size for " + std::string(filename) + ": " + error.what());
                return nullptr;
            }
        }
        if (!initialize_nam_model(next.get(), host_sample_rate, *model_sample_rate,
                                  module_name, filename)) {
            return nullptr;
        }
        return next;
    } catch (const std::exception& error) {
        gx_print_info(module_name, "fail to load " + std::string(filename) +
                      ": " + error.what());
    } catch (...) {
        gx_print_info(module_name, "fail to load " + std::string(filename));
    }
    return nullptr;
}

int setup_nam_resampler(gx_resample::FixedRateResampler& smp,
                        int host_sample_rate, int model_sample_rate) {
    if (model_sample_rate > host_sample_rate) {
        smp.setup(host_sample_rate, model_sample_rate);
        return 1;
    }
    if (model_sample_rate < host_sample_rate) {
        smp.setup(model_sample_rate, host_sample_rate);
        return 2;
    }
    return 0;
}

void reset_nam_filelist(std::vector<Glib::ustring>& names) {
    names.clear();
    names.reserve(kNamSelectorSlots);
    for (int i = 0; i < kNamSelectorSlots; ++i) {
        names.push_back(kNamNone);
    }
}

void populate_nam_filelist(const Glib::ustring& load_path,
                           std::vector<Glib::ustring>& names) {
    reset_nam_filelist(names);
    if (load_path.empty()) {
        return;
    }

    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(load_path);
    if (!file->query_exists()) {
        return;
    }

    Glib::RefPtr<Gio::FileEnumerator> child_enumeration =
          file->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_NAME
                    "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME
                    "," G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
    Glib::RefPtr<Gio::FileInfo> file_info;
    int index = 1;
    while ((file_info = child_enumeration->next_file())) {
        std::string name = file_info->get_name();
        if (name.compare(std::max<int>(0, name.size()-4), 4, ".nam") == 0) {
            names[index++] = file_info->get_name();
            if (index >= kNamSelectorSlots) {
                break;
            }
        }
    }
}

bool get_selected_nam_file(float selector, const Glib::ustring& load_path,
                           const std::vector<Glib::ustring>& names,
                           Glib::ustring* selected_file,
                           const char* module_name) {
    const int index = static_cast<int>(selector);
    if (index <= 0) {
        *selected_file = kNamNone;
        return false;
    }
    if (index >= static_cast<int>(names.size()) || names[index] == kNamNone ||
        load_path.empty()) {
        gx_print_info(module_name, "no NAM model at selector index " +
                      std::to_string(index));
        *selected_file = kNamNone;
        return false;
    }
    *selected_file = load_path + "/" + names[index];
    return true;
}

inline void process_nam_mono(nam::DSP* model, float* input, float* output, int frames) {
    AVOIDDENORMALS();
    float* inputs[] = {input};
    float* outputs[] = {output};
    model->process(inputs, outputs, frames);
}

} // namespace


/****************************************************************
 ** class Ramp small ramp for model switching
 */

inline void Ramp::init(unsigned int rate) {
    mode = OFF;
    ramp_step = 372.0;
    ramp_down = ramp_step;
    ramp_up = 0.0;
    ramp_step_impl = 1.0/ramp_step;
};
    
inline void Ramp::startRampDown() {
    mode = DOWN;
    Glib::signal_timeout().connect_once(
        sigc::mem_fun(*this, &Ramp::checkRampMode), 15);
    
}
    
inline void Ramp::checkRampMode() {
    if (mode != OFF) mode = UP;
}

inline void Ramp::rampDown(int count, float *output) {
    for (int i=0; i<count; i++) {
        if (ramp_down > 0.0) {
            --ramp_down; 
        } else {
            mode = DEAD;
        }
        const float fade = ramp_down * ramp_step_impl ;
        output[i] *= fade;
    }
    ramp_up = ramp_down;
};

inline void Ramp::rampUp(int count, float *output) {
    for (int i=0; i<count; i++) {
        if (ramp_up < ramp_step) {
            ++ramp_up; 
        } else {
            mode = OFF;
        }
        const float fade = ramp_up * ramp_step_impl ;
        output[i] *= fade;
    }
    ramp_down = ramp_up;
};

/****************************************************************
 ** class Neural Amp Modeler
 */

NeuralAmp::NeuralAmp(ParamMap& param_, std::string id_, sigc::slot<void> sync_)
    : PluginDef(), model(nullptr), param(param_), smp(), ramp(), sync(sync_), idstring(id_), plugin() {
    version = PLUGINDEF_VERSION;
    // Resident bypass deliberately freezes the pre-warmed NAM graph and its
    // gain smoothers. ResetAndPrewarm can process 0.5 seconds of model audio
    // for an LSTM, so it must not return to the live scene-switch path.
    flags = PGNI_RESIDENT_PRESERVE_STATE;
    id = idstring.c_str();
    name = N_("Neural Amp Modeler");
    groups = 0;
    description = N_("Neural Amp Modeler by Steven Atkinson"); // description (tooltip)
    category = N_("Neural");       // category
    shortname = (std::strcmp(id, "nam") == 0) ? "NAM II" : "NAM I";     // shortname
    mono_audio = compute_static;
    stereo_audio = 0;
    set_samplerate = init_static;
    activate_plugin = 0;
    register_params = register_params_static;
    load_ui = load_ui_f_static;
    clear_state = clear_state_f_static;
    delete_instance = del_instance;
    plugin = this;
    need_resample = 0;
    is_inited = false;
    loudness = 0.0;
    filelist = 0.0;
    fVslider2 = kDefaultNamSlimmableSize;
    current_model_size = fVslider2;
    nam_input_trim_db = 0.0f;
    nam_output_trim_db = 0.0f;
    gx_system::atomic_set(&ready, 0);
    gx_system::atomic_set(&scene_smoother_snap_pending, 0);
 }

NeuralAmp::~NeuralAmp() {
    delete model;
}

void NeuralAmp::request_scene_smoother_snap() {
    gx_system::atomic_set(&scene_smoother_snap_pending, 1);
}

bool NeuralAmp::finish_scene_smoother_snap() {
    const bool finished = !gx_system::atomic_get(scene_smoother_snap_pending);
    gx_system::atomic_set(&scene_smoother_snap_pending, 0);
    return finished;
}

std::unique_ptr<nam::DSP> NeuralAmp::take_cached_model(
    const Glib::ustring& filename, float size, int* sample_rate) {
    const auto cached = std::find_if(
        model_cache.begin(), model_cache.end(),
        [&](const CachedNamModel& entry) {
            return entry.filename == filename &&
                std::fabs(entry.size - size) < 0.0001f &&
                entry.host_sample_rate == fSampleRate;
        });
    if (cached == model_cache.end()) {
        return nullptr;
    }

    std::unique_ptr<nam::DSP> result = std::move(cached->model);
    if (sample_rate) {
        *sample_rate = cached->sample_rate;
    }
    model_cache.erase(cached);
    return result;
}

void NeuralAmp::cache_current_model() {
    if (!model || current_file.empty() || current_file == kNamNone) {
        delete model;
        model = nullptr;
        return;
    }

    std::unique_ptr<nam::DSP> model_to_cache(model);
    model = nullptr;
    // The outgoing graph was pre-warmed when it was constructed and has just
    // been processing audio. Park it by ownership transfer only. Resetting and
    // processing 0.5 seconds of warm-up audio here made every displacement a
    // synchronous scene-switch cost.

    model_cache.erase(
        std::remove_if(
            model_cache.begin(), model_cache.end(),
            [&](const CachedNamModel& entry) {
                return entry.filename == current_file &&
                    std::fabs(entry.size - current_model_size) < 0.0001f;
            }),
        model_cache.end());
    if (model_cache.size() >= kNamModelCacheEntries) {
        model_cache.erase(model_cache.begin());
    }
    model_cache.push_back(CachedNamModel{
        current_file,
        current_model_size,
        mSampleRate,
        fSampleRate,
        std::move(model_to_cache),
    });
}

inline void NeuralAmp::clear_state_f()
{
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec0[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0;
}

void NeuralAmp::clear_state_f_static(PluginDef *p)
{
    static_cast<NeuralAmp*>(p)->clear_state_f();
}

inline void NeuralAmp::init(unsigned int sample_rate)
{
    fSampleRate = sample_rate;
    clear_state_f();
    reset_nam_filelist(nam_file_names);
    scratch.resize(max_nam_model_block_frames(fSampleRate, fSampleRate));
    ramp.init(fSampleRate);
    is_inited = true;
    load_nam_file();
}

void NeuralAmp::init_static(unsigned int sample_rate, PluginDef *p)
{
    static_cast<NeuralAmp*>(p)->init(sample_rate);
}

void always_inline NeuralAmp::compute(int count, float *input0, float *output0)
{
    if (output0 != input0)
        memcpy(output0, input0, count*sizeof(float));
    if (!model || !gx_system::atomic_get(ready)) return;
    double fSlow0 = 0.0010000000000000009 *
        std::pow(1e+01, 0.05 * double(fVslider0 + nam_input_trim_db));
    double fSlow1 = 0.0010000000000000009 *
        std::pow(1e+01, 0.05 * double(fVslider1 + nam_output_trim_db));
    if (gx_system::atomic_get(scene_smoother_snap_pending)) {
        const double input_target = 1000.0 * fSlow0;
        const double output_target = 1000.0 * fSlow1;
        fRec0[0] = fRec0[1] = input_target;
        fRec1[0] = fRec1[1] = output_target;
        gx_system::atomic_set(&scene_smoother_snap_pending, 0);
    }
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec0[0] = fSlow0 + 0.999 * fRec0[1];
        output0[i0] = float(double(output0[i0]) * fRec0[0]);
        fRec0[1] = fRec0[0];
    }
    if (model && gx_system::atomic_get(ready)) {
        if (need_resample) {
            int ReCount = count;
            if (need_resample == 1) {
                ReCount = smp.max_out_count(count);
            } else if (need_resample == 2) {
                ReCount = static_cast<int>(ceil((count*static_cast<double>(mSampleRate))/fSampleRate));
            }

            if (static_cast<size_t>(ReCount) <= scratch.size()) {
                float* buf = scratch.data();
                memset(buf, 0, ReCount*sizeof(float));

                if (need_resample == 1) {
                    ReCount = smp.up(count, output0, buf);
                } else if (need_resample == 2) {
                    smp.down(output0, buf);
                } else {
                    memcpy(buf, output0, ReCount * sizeof(float));
                }

                process_nam_mono(model, buf, buf, ReCount);

                if (need_resample == 1) {
                    smp.down(buf, output0);
                } else if (need_resample == 2) {
                    smp.up(ReCount, buf, output0);
                }
            }
        } else {
            process_nam_mono(model, output0, output0, count);
        }
    }
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec1[0] = fSlow1 + 0.999 * fRec1[1];
        output0[i0] = float(double(output0[i0]) * fRec1[0]);
        fRec1[1] = fRec1[0];
    }
    if (ramp.mode == ramp.DOWN || ramp.mode == ramp.DEAD) ramp.rampDown(count, output0);
    else if (ramp.mode == ramp.UP) ramp.rampUp(count, output0);
}

void NeuralAmp::compute_static(int count, float *input0, float *output0, PluginDef *p)
{
    static_cast<NeuralAmp*>(p)->compute(count, input0, output0);
}

// non rt callback
void NeuralAmp::load_nam_file_impl() {
    if (model && gx_system::atomic_get(ready) && ramp.mode == ramp.OFF) {
        ramp.startRampDown();
    }
    load_nam_file();
}

// non rt callback
void NeuralAmp::load_nam_file() {
    if (is_inited) {
        Glib::ustring selected_file;
        const bool has_selection = get_selected_nam_file(filelist, load_path,
            nam_file_names, &selected_file, "Neural Amp Modeler");

        if (has_selection && model && (current_file.compare(selected_file) == 0)) {
            if (ramp.mode == ramp.DOWN) ramp.mode = ramp.UP;
            return;
        }

        std::unique_ptr<nam::DSP> next_model;
        int next_sample_rate = 0;
        if (has_selection) {
            next_model = take_cached_model(
                selected_file, fVslider2, &next_sample_rate);
            if (next_model) {
                gx_print_info(
                    "Neural Amp Modeler",
                    "reused prewarmed cached " + std::string(selected_file));
            }
            if (!next_model) {
                next_model = load_nam_model(selected_file, fSampleRate,
                                            &next_sample_rate, "Neural Amp Modeler",
                                            fVslider2, "model");
            }
        }

        ramp.mode = ramp.DOWN;
        gx_system::atomic_set(&ready, 0);
        sync();

        cache_current_model();
        need_resample = 0;
        loudness = 0.0;
        nam_input_trim_db = 0.0f;
        nam_output_trim_db = 0.0f;
        load_file = kNamNone;
        current_file.clear();
        clear_state_f();

        if (next_model) {
            model = next_model.release();
            mSampleRate = next_sample_rate;
            scratch.resize(max_nam_model_block_frames(fSampleRate, mSampleRate));
            need_resample = setup_nam_resampler(smp, fSampleRate, mSampleRate);
            load_file = selected_file;
            current_file = selected_file;
            current_model_size = fVslider2;
            update_nam_level_calibration(
                model, &nam_input_trim_db, &nam_output_trim_db, &loudness,
                "Neural Amp Modeler", selected_file, "model");
            gx_print_info("Neural Amp Modeler", "loaded " + std::string(selected_file));
        }
        gx_system::atomic_set(&ready, model ? 1 : 0);
    }
    ramp.mode = model ? ramp.UP : ramp.OFF;
}

void NeuralAmp::set_nam_size() {
    if (!model || !nam_model_is_slimmable(model)) {
        return;
    }

    if (gx_system::atomic_get(ready) && ramp.mode == ramp.OFF) {
        ramp.startRampDown();
    }
    gx_system::atomic_set(&ready, 0);
    sync();

    if (set_nam_slimmable_size(model, fVslider2, fSampleRate, mSampleRate,
                               "Neural Amp Modeler", current_file, "model")) {
        current_model_size = fVslider2;
        update_nam_level_calibration(
            model, &nam_input_trim_db, &nam_output_trim_db, &loudness,
            "Neural Amp Modeler", current_file, "model");
    }
    gx_system::atomic_set(&ready, model ? 1 : 0);
    ramp.mode = model ? ramp.UP : ramp.OFF;
}

// non rt callback
void NeuralAmp::create_nam_filelist() {
    populate_nam_filelist(load_path, nam_file_names);
    load_nam_file_impl();
}

int NeuralAmp::register_par(const ParamReg& reg)
{
    reg.registerFloatVar((idstring + ".input").c_str(),N_("Input"),"S",N_("gain (dB)"),&fVslider0, 0.0, -40.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".output").c_str(),N_("Output"),"S",N_("gain (dB)"),&fVslider1, 0.0, -40.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".size").c_str(),N_("Size"),"S",N_("slimmable NAM model size"),&fVslider2, kDefaultNamSlimmableSize, 0.0, 1.0, 0.01, 0);
    param.reg_string((idstring + ".loadpath").c_str(), "", &load_path, "", true)->set_desc(N_("load path for *.nam files"));
    param.reg_string((idstring + ".loadfile").c_str(), "", &load_file, "*.nam", true)->set_desc(N_("import *.nam file"));
    reg.registerFloatVar((idstring + ".flist").c_str(),N_("select NAM File"),"S",N_("Select NAM file"),&filelist, 0, 0, 127, 1, 0);

    param[(idstring + ".loadpath").c_str()].signal_changed_string().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmp::create_nam_filelist)));
    param[(idstring + ".flist").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmp::load_nam_file_impl)));
    param[(idstring + ".size").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmp::set_nam_size)));
    
//    param[(idstring + ".loadfile").c_str()].signal_changed_string().connect(
//        sigc::hide(sigc::mem_fun(this, &NeuralAmp::load_nam_file)));
    return 0;
}

int NeuralAmp::register_params_static(const ParamReg& reg)
{
    return static_cast<NeuralAmp*>(reg.plugin)->register_par(reg);
}

inline int NeuralAmp::load_ui_f(const UiBuilder& b, int form)
{
    if (form & UI_FORM_GLADE) {
        b.load_glade_file((idstring + "_ui.glade").c_str());
        return 0;
    }
    if (form & UI_FORM_STACK) {

        b.openHorizontalhideBox("");
            b.create_master_slider((idstring + ".input").c_str(), "Input");
        b.closeBox();
        b.openHorizontalBox("");

            b.create_mid_rackknob((idstring + ".input").c_str(), "Input");
            b.create_fload_switch(sw_button, nullptr, (idstring + ".loadfile").c_str());
            b.create_mid_rackknob((idstring + ".size").c_str(), "Size");
            b.create_mid_rackknob((idstring + ".output").c_str(), "Output");

        b.closeBox();

        return 0;
    }
    return -1;
}

int NeuralAmp::load_ui_f_static(const UiBuilder& b, int form)
{
    return static_cast<NeuralAmp*>(b.plugin)->load_ui_f(b, form);
}

void NeuralAmp::del_instance(PluginDef *p)
{
    delete static_cast<NeuralAmp*>(p);
}

/****************************************************************
 ** class NeuralAmpMulti
 */

NeuralAmpMulti::NeuralAmpMulti(ParamMap& param_, std::string id_, ParallelThread* pro_, sigc::slot<void> sync_)
    : PluginDef(), modela(nullptr), modelb(nullptr), param(param_), pro(pro_), smpa(), smpb(), rampA(), rampB(), sync(sync_), idstring(id_), plugin() {
    version = PLUGINDEF_VERSION;
    // As above, preserve both A/B graphs and wrapper smoothers across a
    // resident bypass. Changed scene gains still settle normally; unchanged
    // gains resume immediately without a hidden zero-to-target ramp.
    flags = PGNI_RESIDENT_PRESERVE_STATE;
    id = idstring.c_str();
    name = N_("Neural Multi Amp Modeler");
    groups = 0;
    description = N_("Neural Amp Modeler by Steven Atkinson"); // description (tooltip)
    category = N_("Neural");       // category
    shortname = "NAM Multi";     // shortname
    mono_audio = compute_static;
    stereo_audio = 0;
    set_samplerate = init_static;
    activate_plugin = 0;
    register_params = register_params_static;
    load_ui = load_ui_f_static;
    clear_state = clear_state_f_static;
    delete_instance = del_instance;
    plugin = this;
    loudnessa = 0.0;
    loudnessb = 0.0;
    nam_input_trim_dba = 0.0f;
    nam_output_trim_dba = 0.0f;
    nam_input_trim_dbb = 0.0f;
    nam_output_trim_dbb = 0.0f;
    need_aresample = 0;
    need_bresample = 0;
    maSampleRate = 0;
    mbSampleRate = 0;
    is_inited = false;
    afilelist = 0.0;
    bfilelist = 0.0;
    fVslider3 = kDefaultNamSlimmableSize;
    fVslider4 = kDefaultNamSlimmableSize;
    current_model_sizea = fVslider3;
    current_model_sizeb = fVslider4;
    gx_system::atomic_set(&ready, 0);
    gx_system::atomic_set(&scene_smoother_snap_pending, 0);
 }

NeuralAmpMulti::~NeuralAmpMulti() {
    delete modela;
    delete modelb;
}

void NeuralAmpMulti::request_scene_smoother_snap() {
    gx_system::atomic_set(&scene_smoother_snap_pending, 1);
}

bool NeuralAmpMulti::finish_scene_smoother_snap() {
    const bool finished = !gx_system::atomic_get(scene_smoother_snap_pending);
    gx_system::atomic_set(&scene_smoother_snap_pending, 0);
    return finished;
}

std::unique_ptr<nam::DSP> NeuralAmpMulti::take_cached_model(
    char slot, const Glib::ustring& filename, float size, int* sample_rate) {
    const auto cached = std::find_if(
        model_cache.begin(), model_cache.end(),
        [&](const CachedNamModel& entry) {
            return entry.slot == slot && entry.filename == filename &&
                std::fabs(entry.size - size) < 0.0001f &&
                entry.host_sample_rate == fSampleRate;
        });
    if (cached == model_cache.end()) {
        return nullptr;
    }

    std::unique_ptr<nam::DSP> result = std::move(cached->model);
    if (sample_rate) {
        *sample_rate = cached->sample_rate;
    }
    model_cache.erase(cached);
    return result;
}

bool NeuralAmpMulti::prepared_key_matches(
    char slot, const Glib::ustring& filename, float size) const {
    return std::find_if(
        prepared_models.begin(), prepared_models.end(),
        [&](const PreparedNamModelDescriptor& descriptor) {
            return descriptor.slot == slot &&
                descriptor.filename == filename &&
                std::fabs(descriptor.size - size) < 0.0001f;
        }) != prepared_models.end();
}

void NeuralAmpMulti::cache_model(
    nam::DSP*& active_model, const Glib::ustring& filename, float size,
    int sample_rate, char slot) {
    if (!active_model || filename.empty() || filename == kNamNone) {
        delete active_model;
        active_model = nullptr;
        return;
    }

    std::unique_ptr<nam::DSP> model_to_cache(active_model);
    active_model = nullptr;
    // This graph was pre-warmed at construction and has just been processing
    // live audio. Parking is deliberately constant-time; song preparation
    // constructs replacement graphs before performance instead of running a
    // half-second model warm-up here.
    model_cache.erase(
        std::remove_if(
            model_cache.begin(), model_cache.end(),
            [&](const CachedNamModel& entry) {
                return entry.slot == slot && entry.filename == filename &&
                    std::fabs(entry.size - size) < 0.0001f;
            }),
        model_cache.end());
    if (model_cache.size() >= kNamModelCacheEntries) {
        const auto unpinned = std::find_if(
            model_cache.begin(), model_cache.end(),
            [&](const CachedNamModel& entry) {
                return !prepared_key_matches(entry.slot, entry.filename, entry.size);
            });
        model_cache.erase(
            unpinned != model_cache.end() ? unpinned : model_cache.begin());
    }

    model_cache.push_back(CachedNamModel{
        slot,
        filename,
        size,
        sample_rate,
        fSampleRate,
        prepared_key_matches(slot, filename, size)
            ? prepared_generation : Glib::ustring(),
        std::move(model_to_cache),
    });
}

PreparedNamModelResult NeuralAmpMulti::prepare_song_models(
    const Glib::ustring& generation,
    const std::vector<PreparedNamModelDescriptor>& descriptors) {
    PreparedNamModelResult result = {
        0, 0, 0, 0, static_cast<int>(kNamModelCacheEntries),
        fSampleRate, false,
    };
    if (!is_inited || fSampleRate <= 0) {
        return result;
    }

    std::vector<PreparedNamModelDescriptor> unique;
    for (const PreparedNamModelDescriptor& descriptor : descriptors) {
        const bool duplicate = std::find_if(
            unique.begin(), unique.end(),
            [&](const PreparedNamModelDescriptor& existing) {
                return existing.slot == descriptor.slot &&
                    existing.filename == descriptor.filename &&
                    std::fabs(existing.size - descriptor.size) < 0.0001f;
            }) != unique.end();
        if (!duplicate) {
            unique.push_back(descriptor);
        }
    }

    prepared_generation = generation;
    prepared_models = unique;
    result.requested = static_cast<int>(unique.size());
    model_cache.erase(
        std::remove_if(
            model_cache.begin(), model_cache.end(),
            [&](const CachedNamModel& entry) {
                return entry.host_sample_rate != fSampleRate;
            }),
        model_cache.end());

    for (const PreparedNamModelDescriptor& descriptor : unique) {
        const bool active_hit =
            (descriptor.slot == 'A' && modela &&
             current_afile == descriptor.filename &&
             std::fabs(current_model_sizea - descriptor.size) < 0.0001f) ||
            (descriptor.slot == 'B' && modelb &&
             current_bfile == descriptor.filename &&
             std::fabs(current_model_sizeb - descriptor.size) < 0.0001f);
        if (active_hit) {
            ++result.active_hits;
            continue;
        }

        const auto cached = std::find_if(
            model_cache.begin(), model_cache.end(),
            [&](const CachedNamModel& entry) {
                return entry.slot == descriptor.slot &&
                    entry.filename == descriptor.filename &&
                    std::fabs(entry.size - descriptor.size) < 0.0001f &&
                    entry.host_sample_rate == fSampleRate;
            });
        if (cached != model_cache.end()) {
            cached->pin_generation = generation;
            ++result.cache_hits;
            continue;
        }

        if (model_cache.size() >= kNamModelCacheEntries) {
            const auto stale = std::find_if(
                model_cache.begin(), model_cache.end(),
                [&](const CachedNamModel& entry) {
                    return !prepared_key_matches(
                        entry.slot, entry.filename, entry.size);
                });
            if (stale == model_cache.end()) {
                continue;
            }
            model_cache.erase(stale);
        }

        int model_sample_rate = 0;
        std::unique_ptr<nam::DSP> prepared = load_nam_model(
            descriptor.filename, fSampleRate, &model_sample_rate,
            "Neural Multi Amp Modeler", descriptor.size,
            descriptor.slot == 'A' ? "prepared A" : "prepared B");
        if (!prepared) {
            continue;
        }
        model_cache.push_back(CachedNamModel{
            descriptor.slot,
            descriptor.filename,
            descriptor.size,
            model_sample_rate,
            fSampleRate,
            generation,
            std::move(prepared),
        });
        ++result.loaded;
    }

    result.rapid_switch_ready =
        result.active_hits + result.cache_hits + result.loaded == result.requested;
    return result;
}

inline void NeuralAmpMulti::clear_state_f()
{
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec0[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec01[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec2[l0] = 0.0;
    for (int l0 = 0; l0 < 16384; l0 = l0 + 1) fDec0[l0] = 0.0;
    for (int l1 = 0; l1 < 2; l1 = l1 + 1) fDel4[l1] = 0.0;
    for (int l2 = 0; l2 < 2; l2 = l2 + 1) fDel0[l2] = 0.0;
    for (int l3 = 0; l3 < 2; l3 = l3 + 1) fDel1[l3] = 0.0;
    for (int l4 = 0; l4 < 2; l4 = l4 + 1) fDel2[l4] = 0.0;
    for (int l5 = 0; l5 < 2; l5 = l5 + 1) fDel3[l5] = 0.0;
}

void NeuralAmpMulti::clear_state_f_static(PluginDef *p)
{
    static_cast<NeuralAmpMulti*>(p)->clear_state_f();
}

inline void NeuralAmpMulti::init(unsigned int sample_rate)
{
    fSampleRate = sample_rate;
    clear_state_f();
    reset_nam_filelist(nam_afile_names);
    reset_nam_filelist(nam_bfile_names);
    scratcha.resize(kMaxNamHostBlockFrames);
    scratchb.resize(kMaxNamHostBlockFrames);
    scratch_modela.resize(max_nam_model_block_frames(fSampleRate, fSampleRate));
    scratch_modelb.resize(max_nam_model_block_frames(fSampleRate, fSampleRate));
    is_inited = true;
    buf = nullptr;
    IOTA0 = 0;
    nframes = 1;
    rampA.init(fSampleRate);
    rampB.init(fSampleRate);
    load_nam_afile();
    load_nam_bfile();
    pro->set<1, NeuralAmpMulti, &NeuralAmpMulti::processModelB>(this);
    
}

void NeuralAmpMulti::init_static(unsigned int sample_rate, PluginDef *p)
{
    static_cast<NeuralAmpMulti*>(p)->init(sample_rate);
}

void always_inline NeuralAmpMulti::processDelay(int count, float *buf)
{
    double fSlow0 = 0.0010000000000000009 * double(fVslider02);
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        double fTemp0 = double(buf[i0]);
        fDec0[IOTA0 & 16383] = fTemp0;
        fDel4[0] = fSlow0 + 0.999 * fDel4[1];
        double fTemp1 = ((fDel0[1] != 0.0) ? (((fDel1[1] > 0.0) & (fDel1[1] < 1.0)) ? fDel0[1] : 0.0) : (((fDel1[1] == 0.0) & (fDel4[0] != fDel2[1])) ? 0.0009765625 : (((fDel1[1] == 1.0) & (fDel4[0] != fDel3[1])) ? -0.0009765625 : 0.0)));
        fDel0[0] = fTemp1;
        fDel1[0] = std::max<double>(0.0, std::min<double>(1.0, fDel1[1] + fTemp1));
        fDel2[0] = (((fDel1[1] >= 1.0) & (fDel3[1] != fDel4[0])) ? fDel4[0] : fDel2[1]);
        fDel3[0] = (((fDel1[1] <= 0.0) & (fDel2[1] != fDel4[0])) ? fDel4[0] : fDel3[1]);
        double fTemp2 = fDec0[(IOTA0 - int(std::min<double>(8192.0, std::max<double>(0.0, fDel2[0])))) & 16383];
        buf[i0] = float(fTemp2 + fDel1[0] * (fDec0[(IOTA0 - int(std::min<double>(8192.0, std::max<double>(0.0, fDel3[0])))) & 16383] - fTemp2));
        IOTA0 = IOTA0 + 1;
        fDel4[1] = fDel4[0];
        fDel0[1] = fDel0[0];
        fDel1[1] = fDel1[0];
        fDel2[1] = fDel2[0];
        fDel3[1] = fDel3[0];
    }
}

void always_inline NeuralAmpMulti::processModelA(int count, float *bufa) {
    if (!modela ) return;
    double fSlow0 = 0.0010000000000000009 *
        std::pow(1e+01, 0.05 * double(fVslider0 + nam_input_trim_dba));
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec0[0] = fSlow0 + 0.999 * fRec0[1];
        bufa[i0] = float(double(bufa[i0]) * fRec0[0]);
        fRec0[1] = fRec0[0];
    }

    if (modela && gx_system::atomic_get(ready)) {
        if (need_aresample) {
            int ReCounta = count;
            if (need_aresample == 1) {
                ReCounta = smpa.max_out_count(count);
            } else if (need_aresample == 2) {
                ReCounta = static_cast<int>(ceil((count*static_cast<double>(maSampleRate))/fSampleRate));
            }

            if (static_cast<size_t>(ReCounta) <= scratch_modela.size()) {
                float* bufa1 = scratch_modela.data();
                memset(bufa1, 0, ReCounta*sizeof(float));

                if (need_aresample == 1) {
                    ReCounta = smpa.up(count, bufa, bufa1);
                } else if (need_aresample == 2) {
                    smpa.down(bufa, bufa1);
                } else {
                    memcpy(bufa1, bufa, ReCounta * sizeof(float));
                }

                process_nam_mono(modela, bufa1, bufa1, ReCounta);

                if (need_aresample == 1) {
                    smpa.down(bufa1, bufa);
                } else if (need_aresample == 2) {
                    smpa.up(ReCounta, bufa1, bufa);
                }
            }
        } else {
            process_nam_mono(modela, bufa, bufa, count);
        }
        const double calibration_gain =
            std::pow(1e+01, 0.05 * double(nam_output_trim_dba));
        for (int i0 = 0; i0 < count; ++i0) {
            bufa[i0] = float(double(bufa[i0]) * calibration_gain);
        }
        if (rampA.mode == rampA.DOWN || rampA.mode == rampA.DEAD) rampA.rampDown(count, bufa);
        else if (rampA.mode == rampA.UP) rampA.rampUp(count, bufa);
    }
}

void always_inline NeuralAmpMulti::processModelB() {
    if (!modelb) return;
    double fSlow01 = 0.0010000000000000009 *
        std::pow(1e+01, 0.05 * double(fVslider01 + nam_input_trim_dbb));
    for (int i0 = 0; i0 < nframes; i0 = i0 + 1) {
        fRec01[0] = fSlow01 + 0.999 * fRec01[1];
        buf[i0] = float(double(buf[i0]) * fRec01[0]);
        fRec01[1] = fRec01[0];
    }

    if (modelb && gx_system::atomic_get(ready)) {
        if (need_bresample) {
            int ReCountb = nframes;
            if (need_bresample == 1) {
                ReCountb = smpb.max_out_count(nframes);
            } else if (need_bresample == 2) {
                ReCountb = static_cast<int>(ceil((nframes*static_cast<double>(mbSampleRate))/fSampleRate));
            }

            if (static_cast<size_t>(ReCountb) <= scratch_modelb.size()) {
                float* buf1 = scratch_modelb.data();
                memset(buf1, 0, ReCountb*sizeof(float));

                if (need_bresample == 1) {
                    ReCountb = smpb.up(nframes, buf, buf1);
                } else if (need_bresample == 2) {
                    smpb.down(buf, buf1);
                } else {
                    memcpy(buf1, buf, ReCountb * sizeof(float));
                }

                process_nam_mono(modelb, buf1, buf1, ReCountb);

                if (need_bresample == 1) {
                    smpb.down(buf1, buf);
                } else if (need_bresample == 2) {
                    smpb.up(ReCountb, buf1, buf);
                }
            }
        } else {
            process_nam_mono(modelb, buf, buf, nframes);
        }
        const double calibration_gain =
            std::pow(1e+01, 0.05 * double(nam_output_trim_dbb));
        for (int i0 = 0; i0 < nframes; ++i0) {
            buf[i0] = float(double(buf[i0]) * calibration_gain);
        }
        if (rampB.mode == rampB.DOWN || rampB.mode == rampB.DEAD) rampB.rampDown(nframes, buf);
        else if (rampB.mode == rampB.UP) rampB.rampUp(nframes, buf);
    }
}


void always_inline NeuralAmpMulti::compute(int count, float *input0, float *output0)
{
    if (output0 != input0)
        memcpy(output0, input0, count*sizeof(float));
    if (!modela && !modelb) return;
    double fSlow1 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider1));
    double fSlow2 = 0.0010000000000000009 * double(fVslider2);

    if (gx_system::atomic_get(scene_smoother_snap_pending)) {
        const double input_a_target = std::pow(
            1e+01, 0.05 * double(fVslider0 + nam_input_trim_dba));
        const double input_b_target = std::pow(
            1e+01, 0.05 * double(fVslider01 + nam_input_trim_dbb));
        const double output_target = std::pow(
            1e+01, 0.05 * double(fVslider1));
        const double mix_target = std::max<double>(
            0.0, std::min<double>(1.0, double(fVslider2)));
        fRec0[0] = fRec0[1] = input_a_target;
        fRec01[0] = fRec01[1] = input_b_target;
        fRec1[0] = fRec1[1] = output_target;
        fRec2[0] = fRec2[1] = mix_target;
        gx_system::atomic_set(&scene_smoother_snap_pending, 0);
    }

    if (static_cast<size_t>(count) > scratcha.size() ||
        static_cast<size_t>(count) > scratchb.size()) {
        return;
    }

    constexpr double kMixEndpointEpsilon = 1.0e-4;
    const double mix_target = std::max<double>(0.0, std::min<double>(1.0, double(fVslider2)));
    const bool mix_settled_a = mix_target <= kMixEndpointEpsilon &&
                               fRec2[1] <= kMixEndpointEpsilon;
    const bool mix_settled_b = mix_target >= 1.0 - kMixEndpointEpsilon &&
                               fRec2[1] >= 1.0 - kMixEndpointEpsilon;
    const bool process_a_only = modela && (!modelb || mix_settled_a);
    const bool process_b_only = modelb && (!modela || mix_settled_b);

    if (process_a_only) {
        float* bufa = scratcha.data();
        memcpy(bufa, output0, count*sizeof(float));
        if (int(fVslider02) <= 0) processDelay(count, bufa);
        processModelA(count, bufa);

        if (modela && gx_system::atomic_get(ready)) {
            memcpy(output0, bufa, count*sizeof(float));
        }
        fRec2[0] = fRec2[1] = 0.0;
    } else if (process_b_only) {
        float* bufb = scratchb.data();
        memcpy(bufb, output0, count*sizeof(float));
        if (int(fVslider02) > 0) processDelay(count, bufb);

        nframes = count;
        buf = bufb;
        processModelB();

        if (modelb && gx_system::atomic_get(ready)) {
            memcpy(output0, bufb, count*sizeof(float));
        }
        fRec2[0] = fRec2[1] = 1.0;
    } else {
        float* bufa = scratcha.data();
        memcpy(bufa, output0, count*sizeof(float));
        float* bufb = scratchb.data();
        memcpy(bufb, output0, count*sizeof(float));

        if (int(fVslider02) > 0) processDelay(count, bufb);
        else processDelay(count, bufa);

        nframes = count;
        buf = bufb;

        if (pro->getProcess()) {
            pro->setProcessor(1);
            pro->runProcess();
        } else {
            processModelB();
        }

        processModelA(count, bufa);

        pro->processWait();

        if (modela && modelb && gx_system::atomic_get(ready)) {
            for (int i0 = 0; i0 < count; i0 = i0 + 1) {
                fRec2[0] = fSlow2 + 0.999 * fRec2[1];
                output0[i0] = bufa[i0] * (1.0 - fRec2[0]) + bufb[i0] * fRec2[0];
                fRec2[1] = fRec2[0];
            }
        } else if (modela && gx_system::atomic_get(ready)) {
            memcpy(output0, bufa, count*sizeof(float));
        } else if (modelb && gx_system::atomic_get(ready)) {
            memcpy(output0, bufb, count*sizeof(float));
        }
    }
    
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec1[0] = fSlow1 + 0.999 * fRec1[1];
        output0[i0] = float(double(output0[i0]) * fRec1[0]);
        fRec1[1] = fRec1[0];
    }
}

void NeuralAmpMulti::compute_static(int count, float *input0, float *output0, PluginDef *p)
{
    static_cast<NeuralAmpMulti*>(p)->compute(count, input0, output0);
}

// non rt callback
void NeuralAmpMulti::load_nam_afile_impl() {
    if (modela && gx_system::atomic_get(ready) && rampA.mode == rampA.OFF) {
        rampA.startRampDown();
    }
    load_nam_afile();
}

// non rt callback
void NeuralAmpMulti::load_nam_afile() {
    if (is_inited) {
        Glib::ustring selected_file;
        const bool has_selection = get_selected_nam_file(afilelist, load_apath,
            nam_afile_names, &selected_file, "Neural Multi Amp Modeler");

        if (has_selection && modela && (current_afile.compare(selected_file) == 0)) {
            if (rampA.mode == rampA.DOWN) rampA.mode = rampA.UP;
            return;
        }

        std::unique_ptr<nam::DSP> next_model;
        int next_sample_rate = 0;
        if (has_selection) {
            next_model = take_cached_model(
                'A', selected_file, fVslider3, &next_sample_rate);
            if (next_model) {
                gx_print_info(
                    "Neural Multi Amp Modeler",
                    "reused prewarmed cached A " + std::string(selected_file));
            }
            if (!next_model) {
                next_model = load_nam_model(selected_file, fSampleRate,
                                            &next_sample_rate,
                                            "Neural Multi Amp Modeler",
                                            fVslider3, "A");
            }
        }

        rampA.mode = rampA.DOWN;
        gx_system::atomic_set(&ready, 0);
        sync();

        cache_model(
            modela, current_afile, current_model_sizea, maSampleRate, 'A');
        need_aresample = 0;
        loudnessa = 0.0;
        nam_input_trim_dba = 0.0f;
        nam_output_trim_dba = 0.0f;
        load_afile = kNamNone;
        current_afile.clear();
        clear_state_f();

        if (next_model) {
            modela = next_model.release();
            maSampleRate = next_sample_rate;
            scratch_modela.resize(max_nam_model_block_frames(fSampleRate, maSampleRate));
            need_aresample = setup_nam_resampler(smpa, fSampleRate, maSampleRate);
            load_afile = selected_file;
            current_afile = selected_file;
            current_model_sizea = fVslider3;
            update_nam_level_calibration(
                modela, &nam_input_trim_dba, &nam_output_trim_dba, &loudnessa,
                "Neural Multi Amp Modeler", selected_file, "A");
            gx_print_info("Neural Multi Amp Modeler", "loaded A " + std::string(selected_file));
        }
        gx_system::atomic_set(&ready, (modela || modelb) ? 1 : 0);
    }
    rampA.mode = modela ? rampA.UP : rampA.OFF;
}

// non rt callback
void NeuralAmpMulti::load_nam_bfile_impl() {
    if (modelb && gx_system::atomic_get(ready) && rampB.mode == rampB.OFF) {
        rampB.startRampDown();
    }
    load_nam_bfile();
}

// non rt callback
void NeuralAmpMulti::load_nam_bfile() {
    if (is_inited) {
        Glib::ustring selected_file;
        const bool has_selection = get_selected_nam_file(bfilelist, load_bpath,
            nam_bfile_names, &selected_file, "Neural Multi Amp Modeler");

        if (has_selection && modelb && (current_bfile.compare(selected_file) == 0)) {
            if (rampB.mode == rampB.DOWN) rampB.mode = rampB.UP;
            return;
        }

        std::unique_ptr<nam::DSP> next_model;
        int next_sample_rate = 0;
        if (has_selection) {
            next_model = take_cached_model(
                'B', selected_file, fVslider4, &next_sample_rate);
            if (next_model) {
                gx_print_info(
                    "Neural Multi Amp Modeler",
                    "reused prewarmed cached B " + std::string(selected_file));
            }
            if (!next_model) {
                next_model = load_nam_model(selected_file, fSampleRate,
                                            &next_sample_rate,
                                            "Neural Multi Amp Modeler",
                                            fVslider4, "B");
            }
        }

        rampB.mode = rampB.DOWN;
        gx_system::atomic_set(&ready, 0);
        sync();

        cache_model(
            modelb, current_bfile, current_model_sizeb, mbSampleRate, 'B');
        need_bresample = 0;
        loudnessb = 0.0;
        nam_input_trim_dbb = 0.0f;
        nam_output_trim_dbb = 0.0f;
        load_bfile = kNamNone;
        current_bfile.clear();
        clear_state_f();

        if (next_model) {
            modelb = next_model.release();
            mbSampleRate = next_sample_rate;
            scratch_modelb.resize(max_nam_model_block_frames(fSampleRate, mbSampleRate));
            need_bresample = setup_nam_resampler(smpb, fSampleRate, mbSampleRate);
            load_bfile = selected_file;
            current_bfile = selected_file;
            current_model_sizeb = fVslider4;
            update_nam_level_calibration(
                modelb, &nam_input_trim_dbb, &nam_output_trim_dbb, &loudnessb,
                "Neural Multi Amp Modeler", selected_file, "B");
            gx_print_info("Neural Multi Amp Modeler", "loaded B " + std::string(selected_file));
        }
        gx_system::atomic_set(&ready, (modela || modelb) ? 1 : 0);
    }
    rampB.mode = modelb ? rampB.UP : rampB.OFF;
}

void NeuralAmpMulti::set_nam_asize() {
    if (!modela || !nam_model_is_slimmable(modela)) {
        return;
    }

    if (gx_system::atomic_get(ready) && rampA.mode == rampA.OFF) {
        rampA.startRampDown();
    }
    gx_system::atomic_set(&ready, 0);
    sync();

    if (set_nam_slimmable_size(modela, fVslider3, fSampleRate, maSampleRate,
                               "Neural Multi Amp Modeler", current_afile, "A")) {
        current_model_sizea = fVslider3;
        update_nam_level_calibration(
            modela, &nam_input_trim_dba, &nam_output_trim_dba, &loudnessa,
            "Neural Multi Amp Modeler", current_afile, "A");
    }
    gx_system::atomic_set(&ready, (modela || modelb) ? 1 : 0);
    rampA.mode = modela ? rampA.UP : rampA.OFF;
}

void NeuralAmpMulti::set_nam_bsize() {
    if (!modelb || !nam_model_is_slimmable(modelb)) {
        return;
    }

    if (gx_system::atomic_get(ready) && rampB.mode == rampB.OFF) {
        rampB.startRampDown();
    }
    gx_system::atomic_set(&ready, 0);
    sync();

    if (set_nam_slimmable_size(modelb, fVslider4, fSampleRate, mbSampleRate,
                               "Neural Multi Amp Modeler", current_bfile, "B")) {
        current_model_sizeb = fVslider4;
        update_nam_level_calibration(
            modelb, &nam_input_trim_dbb, &nam_output_trim_dbb, &loudnessb,
            "Neural Multi Amp Modeler", current_bfile, "B");
    }
    gx_system::atomic_set(&ready, (modela || modelb) ? 1 : 0);
    rampB.mode = modelb ? rampB.UP : rampB.OFF;
}

// non rt callback
void NeuralAmpMulti::create_nam_afilelist() {
    populate_nam_filelist(load_apath, nam_afile_names);
    load_nam_afile_impl();
}

// non rt callback
void NeuralAmpMulti::create_nam_bfilelist() {
    populate_nam_filelist(load_bpath, nam_bfile_names);
    load_nam_bfile_impl();
}

int NeuralAmpMulti::register_par(const ParamReg& reg)
{
    reg.registerFloatVar((idstring + ".input").c_str(),N_("Input A"),"S",N_("gain (dB)"),&fVslider0, 0.0, -20.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".inputb").c_str(),N_("Input B"),"S",N_("gain (dB)"),&fVslider01, 0.0, -20.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".cdelay").c_str(),N_("Delta Delay"),"S",N_("Delay A/B"),&fVslider02, 0.0, -4096.0, 4096.0, 1.0, 0);
    reg.registerFloatVar((idstring + ".output").c_str(),N_("Output"),"S",N_("gain (dB)"),&fVslider1, 0.0, -20.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".mix").c_str(),N_("Mix"),"S",N_("mix models"),&fVslider2, 0.5, 0.0, 1.0, 0.01, 0);
    reg.registerFloatVar((idstring + ".sizea").c_str(),N_("Size A"),"S",N_("slimmable NAM model size A"),&fVslider3, kDefaultNamSlimmableSize, 0.0, 1.0, 0.01, 0);
    reg.registerFloatVar((idstring + ".sizeb").c_str(),N_("Size B"),"S",N_("slimmable NAM model size B"),&fVslider4, kDefaultNamSlimmableSize, 0.0, 1.0, 0.01, 0);
    param.reg_string((idstring + ".loadapath").c_str(), "", &load_apath, "", true)->set_desc(N_("load path for A *.nam files"));
    param.reg_string((idstring + ".loadbpath").c_str(), "", &load_bpath, "", true)->set_desc(N_("load path for B *.nam files"));
    param.reg_string((idstring + ".loadafile").c_str(), "", &load_afile, "*.nam", true)->set_desc(N_("import *.nam file"));
    param.reg_string((idstring + ".loadbfile").c_str(), "", &load_bfile, "*.nam", true)->set_desc(N_("import *.nam file"));
    reg.registerFloatVar((idstring + ".falist").c_str(),N_("select NAM File"),"S",N_("Select NAM file"),&afilelist, 0, 0, 127, 1, 0);
    reg.registerFloatVar((idstring + ".fblist").c_str(),N_("select NAM File"),"S",N_("Select NAM file"),&bfilelist, 0, 0, 127, 1, 0);

    param[(idstring + ".loadapath").c_str()].signal_changed_string().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::create_nam_afilelist)));
    param[(idstring + ".loadbpath").c_str()].signal_changed_string().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::create_nam_bfilelist)));
    param[(idstring + ".falist").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::load_nam_afile_impl)));
    param[(idstring + ".fblist").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::load_nam_bfile_impl)));
    param[(idstring + ".sizea").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::set_nam_asize)));
    param[(idstring + ".sizeb").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::set_nam_bsize)));

//    param[(idstring + ".loadafile").c_str()].signal_changed_string().connect(
//        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::load_nam_afile)));
//    param[(idstring + ".loadbfile").c_str()].signal_changed_string().connect(
//        sigc::hide(sigc::mem_fun(this, &NeuralAmpMulti::load_nam_bfile)));
    return 0;
}

int NeuralAmpMulti::register_params_static(const ParamReg& reg)
{
    return static_cast<NeuralAmpMulti*>(reg.plugin)->register_par(reg);
}

inline int NeuralAmpMulti::load_ui_f(const UiBuilder& b, int form)
{
    if (form & UI_FORM_GLADE) {
        b.load_glade_file((idstring + "_ui.glade").c_str());
        return 0;
    }
    if (form & UI_FORM_STACK) {

        b.openHorizontalhideBox("");
            b.create_master_slider((idstring + ".output").c_str(), "output");
        b.closeBox();
        b.openHorizontalBox("");
            b.openVerticalBox("");
            b.create_mid_rackknob((idstring + ".input").c_str(), "Input A");
            b.create_mid_rackknob((idstring + ".inputb").c_str(), "Input B");
            b.closeBox();
            b.openVerticalBox("");
            b.create_mid_rackknob((idstring + ".sizea").c_str(), "Size A");
            b.create_mid_rackknob((idstring + ".sizeb").c_str(), "Size B");
            b.closeBox();
            b.openVerticalBox("");
                b.create_fload_switch(sw_button, nullptr, (idstring + ".loadafile").c_str());
                b.create_fload_switch(sw_button, nullptr, (idstring + ".loadbfile").c_str());
            b.closeBox();
            b.create_mid_rackknob((idstring + ".output").c_str(), "Output");
            b.create_mid_rackknob((idstring + ".mix").c_str(), "Mix");

        b.closeBox();

        return 0;
    }
    return -1;
}

int NeuralAmpMulti::load_ui_f_static(const UiBuilder& b, int form)
{
    return static_cast<NeuralAmpMulti*>(b.plugin)->load_ui_f(b, form);
}

void NeuralAmpMulti::del_instance(PluginDef *p)
{
    delete static_cast<NeuralAmpMulti*>(p);
}

/****************************************************************
 ** class RtNeural
 */

RtNeural::RtNeural(ParamMap& param_, std::string id_, sigc::slot<void> sync_)
    : PluginDef(), model(nullptr), param(param_), smp(), ramp(), sync(sync_), idstring(id_), plugin() {
    version = PLUGINDEF_VERSION;
    flags = 0;
    id = idstring.c_str();
    name = N_("RTNeural Network Engine");
    groups = 0;
    description = N_("Neural network engine written by Jatin Chowdhury"); // description (tooltip)
    category = N_("Neural");       // category
    shortname = (std::strcmp(id, "rtneural") == 0) ? "RTNeural I" : "RTNeural II"; // shortname
    mono_audio = compute_static;
    stereo_audio = 0;
    set_samplerate = init_static;
    activate_plugin = 0;
    register_params = register_params_static;
    load_ui = load_ui_f_static;
    clear_state = clear_state_f_static;
    delete_instance = del_instance;
    plugin = this;
    need_resample = 0;
    is_inited = false;
    gx_system::atomic_set(&ready, 0);
 }

RtNeural::~RtNeural() {
    delete model;
}

inline void RtNeural::clear_state_f()
{
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec0[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0;
    for (int l0 = 0; l0 < 127; l0 = l0 + 1) rtneural_file_names.push_back("None");
}

void RtNeural::clear_state_f_static(PluginDef *p)
{
    static_cast<RtNeural*>(p)->clear_state_f();
}

inline void RtNeural::init(unsigned int sample_rate)
{
    fSampleRate = sample_rate;
    clear_state_f();
    ramp.init(fSampleRate);
    is_inited = true;
    load_json_file();
}

void RtNeural::init_static(unsigned int sample_rate, PluginDef *p)
{
    static_cast<RtNeural*>(p)->init(sample_rate);
}

void always_inline RtNeural::compute(int count, float *input0, float *output0)
{
    if (output0 != input0)
        memcpy(output0, input0, count*sizeof(float));
    if (!model && !gx_system::atomic_get(ready)) return;
    double fSlow0 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider0));
    double fSlow1 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider1));
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec0[0] = fSlow0 + 0.999 * fRec0[1];
        output0[i0] = float(double(output0[i0]) * fRec0[0]);
        fRec0[1] = fRec0[0];
    }
    if (model && gx_system::atomic_get(ready)) {
        if (need_resample) {
            int ReCount = count;
            if (need_resample == 1) {
                ReCount = smp.max_out_count(count);
            } else if (need_resample == 2) {
                ReCount = static_cast<int>(ceil((count*static_cast<double>(mSampleRate))/fSampleRate));
            }

            float buf[ReCount];
            memset(buf, 0, ReCount*sizeof(float));

            if (need_resample == 1) {
                ReCount = smp.up(count, output0, buf);
            } else if (need_resample == 2) {
                smp.down(output0, buf);
            } else {
                memcpy(buf, output0, ReCount * sizeof(float));
            }

            for (int i0 = 0; i0 < ReCount; i0 = i0 + 1) {
                 buf[i0] = model->forward (&buf[i0]);
            }

            if (need_resample == 1) {
                smp.down(buf, output0);
            } else if (need_resample == 2) {
                smp.up(ReCount, buf, output0);
            }
        } else {
            for (int i0 = 0; i0 < count; i0 = i0 + 1) {
                 output0[i0] = model->forward (&output0[i0]);
            }
        }
    }
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec1[0] = fSlow1 + 0.999 * fRec1[1];
        output0[i0] = float(double(output0[i0]) * fRec1[0]);
        fRec1[1] = fRec1[0];
    }
    if (ramp.mode == ramp.DOWN || ramp.mode == ramp.DEAD) ramp.rampDown(count, output0);
    else if (ramp.mode == ramp.UP) ramp.rampUp(count, output0);
}

void RtNeural::compute_static(int count, float *input0, float *output0, PluginDef *p)
{
    static_cast<RtNeural*>(p)->compute(count, input0, output0);
}

void RtNeural::get_samplerate(std::string config_file) {
    std::ifstream infile(config_file);
    infile.imbue(std::locale::classic());
    std::string line;
    std::string key;
    std::string value;
    if (infile.is_open()) {
        while (std::getline(infile, line)) {
            std::istringstream buf(line);
            buf >> key;
            buf >> value;
            if (key.compare("\"samplerate\":") == 0) {
                value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
                mSampleRate = std::stoi(value);
                break;
            }
            key.clear();
            value.clear();
        }
        infile.close();
    }
}

// non rt callback
void RtNeural::load_json_file_impl() {
    if (ramp.mode == ramp.OFF) ramp.startRampDown();
    Glib::signal_timeout().connect_once(
        sigc::mem_fun(*this, &RtNeural::load_json_file), 3);
}

// non rt callback
void RtNeural::load_json_file() {
    if (is_inited) {
        if (rtneural_file_names.size() < 1 || filelist < 1.0) return;
        load_file = load_path + "/" + rtneural_file_names[filelist];
        if (!current_file.empty() && (current_file.compare(load_file) == 0)) {
            if (ramp.mode == ramp.DOWN) ramp.mode = ramp.UP;
            return;
        }
        ramp.mode = ramp.DOWN;
        gx_system::atomic_set(&ready, 0);
        sync();
        delete model;
        model = nullptr;
        mSampleRate = 0;
        need_resample = 0;
        clear_state_f();
        try {
            get_samplerate(std::string(load_file));
            std::ifstream jsonStream(std::string(load_file), std::ifstream::binary);
            model = RTNeural::json_parser::parseJson<float>(jsonStream).release();
        } catch (const std::exception&) {
            gx_print_info("RTNeural Amp Modeler", "fail to load " + load_file);
            load_file = "None";
        }
        
        if (model) {
            current_file = load_file;
            model->reset();
            if (mSampleRate <= 0) mSampleRate = 48000;
            if (mSampleRate > fSampleRate) {
                smp.setup(fSampleRate, mSampleRate);
                need_resample = 1;
            } else if (mSampleRate < fSampleRate) {
                smp.setup(mSampleRate, fSampleRate);
                need_resample = 2;
            }
        } 
        gx_system::atomic_set(&ready, 1);
    }
    ramp.mode = ramp.UP;
}

// non rt callback
void RtNeural::create_rtneural_filelist() {
    if (load_path.empty()) return;
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(load_path);
    rtneural_file_names.clear();
    int i = 0;
    if (file->query_exists()) {
        Glib::RefPtr<Gio::FileEnumerator> child_enumeration =
              file->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_NAME
                        "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME
                        "," G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
        Glib::RefPtr<Gio::FileInfo> file_info;
        rtneural_file_names.push_back("None");
        while ((file_info = child_enumeration->next_file())) {
            std::string name = file_info->get_name();
            if ((name.compare(std::max<int>(0, name.size()-4), 4, "idax") == 0) ||
                (name.compare(std::max<int>(0, name.size()-4), 4, "json") == 0)) {
                rtneural_file_names.push_back(file_info->get_name());
                i++;
                if ( i > 126) break;
            }
        }
    }
    for (;i<127;i++) rtneural_file_names.push_back("None");
}

int RtNeural::register_par(const ParamReg& reg)
{
    reg.registerFloatVar((idstring + ".input").c_str(),N_("Input"),"S",N_("gain (dB)"),&fVslider0, 0.0, -20.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".output").c_str(),N_("Output"),"S",N_("gain (dB)"),&fVslider1, 0.0, -20.0, 20.0, 0.1, 0);
    param.reg_string((idstring + ".loadpath").c_str(), "", &load_path, "", true)->set_desc(N_("load path for *.json files"));
    param.reg_string((idstring + ".loadfile").c_str(), "", &load_file, "*.json", true)->set_desc(N_("import *.json file"));
    reg.registerFloatVar((idstring + ".flist").c_str(),N_("select json/aidax File"),"S",N_("Select json/aidax file"),&filelist, 0, 0, 127, 1, 0);

    param[(idstring + ".loadpath").c_str()].signal_changed_string().connect(
        sigc::hide(sigc::mem_fun(this, &RtNeural::create_rtneural_filelist)));
    param[(idstring + ".flist").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &RtNeural::load_json_file_impl)));

//    param[(idstring + ".loadfile").c_str()].signal_changed_string().connect(
//        sigc::hide(sigc::mem_fun(this, &RtNeural::load_json_file)));
    return 0;
}

int RtNeural::register_params_static(const ParamReg& reg)
{
    return static_cast<RtNeural*>(reg.plugin)->register_par(reg);
}

inline int RtNeural::load_ui_f(const UiBuilder& b, int form)
{
    if (form & UI_FORM_GLADE) {
        b.load_glade_file((idstring + "_ui.glade").c_str());
        return 0;
    }
    if (form & UI_FORM_STACK) {

        b.openHorizontalhideBox("");
            b.create_master_slider((idstring + "input").c_str(), "Input");
        b.closeBox();
        b.openHorizontalBox("");

            b.create_mid_rackknob((idstring + "input").c_str(), "Input");
            b.create_fload_switch(sw_button, nullptr, (idstring + "loadfile").c_str());
            b.create_mid_rackknob((idstring + "output").c_str(), "Output");

        b.closeBox();

        return 0;
    }
    return -1;
}

int RtNeural::load_ui_f_static(const UiBuilder& b, int form)
{
    return static_cast<RtNeural*>(b.plugin)->load_ui_f(b, form);
}

void RtNeural::del_instance(PluginDef *p)
{
    delete static_cast<RtNeural*>(p);
}

/****************************************************************
 ** class RtNeuralMulti
 */

RtNeuralMulti::RtNeuralMulti(ParamMap& param_, std::string id_, ParallelThread *pro_, sigc::slot<void> sync_)
    : PluginDef(), modela(nullptr), modelb(nullptr), param(param_), pro(pro_), smpa(), smpb(), rampA(), rampB(), sync(sync_), idstring(id_), plugin() {
    version = PLUGINDEF_VERSION;
    flags = 0;
    id = idstring.c_str();
    name = N_("RTNeural Multi Engine");
    groups = 0;
    description = N_("Neural network engine written by Jatin Chowdhury"); // description (tooltip)
    category = N_("Neural");       // category
    shortname = "RTNeuralMulti";     // shortname
    mono_audio = compute_static;
    stereo_audio = 0;
    set_samplerate = init_static;
    activate_plugin = 0;
    register_params = register_params_static;
    load_ui = load_ui_f_static;
    clear_state = clear_state_f_static;
    delete_instance = del_instance;
    plugin = this;
    need_aresample = 0;
    need_bresample = 0;
    is_inited = false;
    gx_system::atomic_set(&ready, 0);
 }

RtNeuralMulti::~RtNeuralMulti() {
    delete modela;
    delete modelb;
}

inline void RtNeuralMulti::clear_state_f()
{
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec0[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec01[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec2[l0] = 0.0;
    for (int l0 = 0; l0 < 16384; l0 = l0 + 1) fDec0[l0] = 0.0;
    for (int l1 = 0; l1 < 2; l1 = l1 + 1) fDel4[l1] = 0.0;
    for (int l2 = 0; l2 < 2; l2 = l2 + 1) fDel0[l2] = 0.0;
    for (int l3 = 0; l3 < 2; l3 = l3 + 1) fDel1[l3] = 0.0;
    for (int l4 = 0; l4 < 2; l4 = l4 + 1) fDel2[l4] = 0.0;
    for (int l5 = 0; l5 < 2; l5 = l5 + 1) fDel3[l5] = 0.0;
    for (int l0 = 0; l0 < 127; l0 = l0 + 1) rtneural_afile_names.push_back("None");
    for (int l0 = 0; l0 < 127; l0 = l0 + 1) rtneural_bfile_names.push_back("None");
}

void RtNeuralMulti::clear_state_f_static(PluginDef *p)
{
    static_cast<RtNeuralMulti*>(p)->clear_state_f();
}

inline void RtNeuralMulti::init(unsigned int sample_rate)
{
    fSampleRate = sample_rate;
    clear_state_f();
    IOTA0 = 0;
    is_inited = true;
    buf = nullptr;
    nframes = 1;
    rampA.init(fSampleRate);
    rampB.init(fSampleRate);
    load_json_afile();
    load_json_bfile();
    pro->set<0, RtNeuralMulti, &RtNeuralMulti::processModelB>(this);
}

void RtNeuralMulti::init_static(unsigned int sample_rate, PluginDef *p)
{
    static_cast<RtNeuralMulti*>(p)->init(sample_rate);
}

void always_inline RtNeuralMulti::processDelay(int count, float *buf)
{
	double fSlow0 = 0.0010000000000000009 * double(fVslider02);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = double(buf[i0]);
		fDec0[IOTA0 & 16383] = fTemp0;
		fDel4[0] = fSlow0 + 0.999 * fDel4[1];
		double fTemp1 = ((fDel0[1] != 0.0) ? (((fDel1[1] > 0.0) & (fDel1[1] < 1.0)) ? fDel0[1] : 0.0) : (((fDel1[1] == 0.0) & (fDel4[0] != fDel2[1])) ? 0.0009765625 : (((fDel1[1] == 1.0) & (fDel4[0] != fDel3[1])) ? -0.0009765625 : 0.0)));
		fDel0[0] = fTemp1;
		fDel1[0] = std::max<double>(0.0, std::min<double>(1.0, fDel1[1] + fTemp1));
		fDel2[0] = (((fDel1[1] >= 1.0) & (fDel3[1] != fDel4[0])) ? fDel4[0] : fDel2[1]);
		fDel3[0] = (((fDel1[1] <= 0.0) & (fDel2[1] != fDel4[0])) ? fDel4[0] : fDel3[1]);
		double fTemp2 = fDec0[(IOTA0 - int(std::min<double>(8192.0, std::max<double>(0.0, fDel2[0])))) & 16383];
		buf[i0] = float(fTemp2 + fDel1[0] * (fDec0[(IOTA0 - int(std::min<double>(8192.0, std::max<double>(0.0, fDel3[0])))) & 16383] - fTemp2));
		IOTA0 = IOTA0 + 1;
		fDel4[1] = fDel4[0];
		fDel0[1] = fDel0[0];
		fDel1[1] = fDel1[0];
		fDel2[1] = fDel2[0];
		fDel3[1] = fDel3[0];
	}
}

void always_inline RtNeuralMulti::processModelA(int count, float *bufa) {
    if (!modela) return;
    double fSlow0 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider0));

    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec0[0] = fSlow0 + 0.999 * fRec0[1];
        bufa[i0] = float(double(bufa[i0]) * fRec0[0]);
        fRec0[1] = fRec0[0];
    }

    if (modela && gx_system::atomic_get(ready)) {
        if (need_aresample ) {
            int ReCounta = count;

            if (need_aresample == 1) {
                ReCounta = smpa.max_out_count(count);
            } else if (need_aresample == 2) {
                ReCounta = static_cast<int>(ceil((count*static_cast<double>(maSampleRate))/fSampleRate));
            }

            float bufa1[ReCounta];
            memset(bufa1, 0, ReCounta*sizeof(float));

            if (need_aresample == 1) {
                ReCounta = smpa.up(count, bufa, bufa1);
            } else if (need_aresample == 2) {
                smpa.down(bufa, bufa1);
            } else {
                memcpy(bufa1, bufa, ReCounta * sizeof(float));
            }

            for (int i0 = 0; i0 < ReCounta; i0 = i0 + 1) {
                 bufa1[i0] = modela->forward (&bufa1[i0]);
            }

            if (need_aresample == 1) {
                smpa.down(bufa1, bufa);
            } else if (need_aresample == 2) {
                smpa.up(ReCounta, bufa1, bufa);
            }
        } else {
            for (int i0 = 0; i0 < count; i0 = i0 + 1) {
                 bufa[i0] = modela->forward (&bufa[i0]);
            }
        }
        if (rampA.mode == rampA.DOWN || rampA.mode == rampA.DEAD) rampA.rampDown(count, bufa);
        else if (rampA.mode == rampA.UP) rampA.rampUp(count, bufa);
    }
}

void always_inline RtNeuralMulti::processModelB() {
    if (!modelb) return;
    double fSlow01 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider01));

    for (int i0 = 0; i0 < nframes; i0 = i0 + 1) {
        fRec01[0] = fSlow01 + 0.999 * fRec01[1];
        buf[i0] = float(double(buf[i0]) * fRec01[0]);
        fRec01[1] = fRec01[0];
    }
    if (modelb && gx_system::atomic_get(ready)) {
        if (need_bresample) {
            int ReCountb = nframes;

            if (need_bresample == 1) {
                ReCountb = smpb.max_out_count(nframes);
            } else if (need_bresample == 2) {
                ReCountb = static_cast<int>(ceil((nframes*static_cast<double>(mbSampleRate))/fSampleRate));
            }

            float buf1[ReCountb];
            memset(buf1, 0, ReCountb*sizeof(float));

            if (need_bresample == 1) {
                ReCountb = smpb.up(nframes, buf, buf1);
            } else if (need_bresample == 2) {
                smpb.down(buf, buf1);
            } else {
                memcpy(buf1, buf, ReCountb * sizeof(float));
            }

            for (int i0 = 0; i0 < ReCountb; i0 = i0 + 1) {
                 buf1[i0] = modelb->forward (&buf1[i0]);
            }

            if (need_bresample == 1) {
                smpb.down(buf1, buf);
            } else if (need_bresample == 2) {
                smpb.up(ReCountb, buf1, buf);
            }
        } else {
            for (int i0 = 0; i0 < nframes; i0 = i0 + 1) {
                 buf[i0] = modelb->forward (&buf[i0]);
            }
        }
        if (rampB.mode == rampB.DOWN || rampB.mode == rampB.DEAD) rampB.rampDown(nframes, buf);
        else if (rampB.mode == rampB.UP) rampB.rampUp(nframes, buf);
    }
}

void always_inline RtNeuralMulti::compute(int count, float *input0, float *output0)
{
    if (output0 != input0)
        memcpy(output0, input0, count*sizeof(float));
    if (!modela || !modelb) return;
    double fSlow1 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider1));
    double fSlow2 = 0.0010000000000000009 * double(fVslider2);

    float bufa[count];
    memcpy(bufa, output0, count*sizeof(float));
    float bufb[count];
    memcpy(bufb, output0, count*sizeof(float));

    if (int(fVslider02) > 0) processDelay(count, bufb);
    else processDelay(count, bufa);

    nframes = count;
    buf = bufb;

    if (pro->getProcess()) {
        pro->setProcessor(0);
        pro->runProcess();
    } else {
        processModelB();
    }

    processModelA(count, bufa);

    pro->processWait();

    if (modela && modelb && gx_system::atomic_get(ready)) {
        for (int i0 = 0; i0 < count; i0 = i0 + 1) {
            fRec2[0] = fSlow2 + 0.999 * fRec2[1];
            output0[i0] = bufa[i0] * (1.0 - fRec2[0]) + bufb[i0] * fRec2[0];
            fRec2[1] = fRec2[0];
        }
    } else if (modela && gx_system::atomic_get(ready)) {
        memcpy(output0, bufa, count*sizeof(float));
    } else if (modelb && gx_system::atomic_get(ready)) {
        memcpy(output0, bufb, count*sizeof(float));
    }
    
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        fRec1[0] = fSlow1 + 0.999 * fRec1[1];
        output0[i0] = float(double(output0[i0]) * fRec1[0]);
        fRec1[1] = fRec1[0];
    }
}

void RtNeuralMulti::compute_static(int count, float *input0, float *output0, PluginDef *p)
{
    static_cast<RtNeuralMulti*>(p)->compute(count, input0, output0);
}

void RtNeuralMulti::get_samplerate(std::string config_file, int *mSampleRate) {
    std::ifstream infile(config_file);
    infile.imbue(std::locale::classic());
    std::string line;
    std::string key;
    std::string value;
    if (infile.is_open()) {
        while (std::getline(infile, line)) {
            std::istringstream buf(line);
            buf >> key;
            buf >> value;
            if (key.compare("\"samplerate\":") == 0) {
                value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
                (*mSampleRate) = std::stoi(value);
                break;
            }
            key.clear();
            value.clear();
        }
        infile.close();
    }
}

// non rt callback
void RtNeuralMulti::load_json_afile_impl() {
    if (rampA.mode == rampA.OFF) rampA.startRampDown();
    Glib::signal_timeout().connect_once(
        sigc::mem_fun(*this, &RtNeuralMulti::load_json_afile), 3);
}

// non rt callback
void RtNeuralMulti::load_json_afile() {
    if (is_inited) {
        if (rtneural_afile_names.size() < 1 || afilelist < 1.0) return;
        load_afile = load_apath + "/" + rtneural_afile_names[afilelist];
        if (!current_afile.empty() && (current_afile.compare(load_afile) == 0)) {
            if (rampA.mode == rampA.DOWN) rampA.mode = rampA.UP;
            return;
        }
        rampA.mode = rampA.DOWN;
        gx_system::atomic_set(&ready, 0);
        sync();
        delete modela;
        modela = nullptr;
        maSampleRate = 0;
        need_aresample = 0;
        clear_state_f();
        try {
            get_samplerate(std::string(load_afile), &maSampleRate);
            std::ifstream jsonStream(std::string(load_afile), std::ifstream::binary);
            modela = RTNeural::json_parser::parseJson<float>(jsonStream).release();
        } catch (const std::exception&) {
            gx_print_info("RTNeural Multi Amp Modeler", "fail to load " + load_afile);
            load_afile = "None";
        }
        
        if (modela) {
            current_afile = load_afile;
            modela->reset();
            if (maSampleRate <= 0) maSampleRate = 48000;
            if (maSampleRate > fSampleRate) {
                smpa.setup(fSampleRate, maSampleRate);
                need_aresample = 1;
            } else if (maSampleRate < fSampleRate) {
                smpa.setup(maSampleRate, fSampleRate);
                need_aresample = 2;
            } 
             //fprintf(stderr, "A: %s\n", load_afile.c_str());
        } 
        gx_system::atomic_set(&ready, 1);
    }
    rampA.mode = rampA.UP;
}

// non rt callback
void RtNeuralMulti::load_json_bfile_impl() {
    if (rampB.mode == rampB.OFF) rampB.startRampDown();
    Glib::signal_timeout().connect_once(
        sigc::mem_fun(*this, &RtNeuralMulti::load_json_bfile), 3);
}

// non rt callback
void RtNeuralMulti::load_json_bfile() {
    if (is_inited) {
        if (rtneural_bfile_names.size() < 1 || bfilelist < 1.0) return;
        load_bfile = load_bpath + "/" + rtneural_bfile_names[bfilelist];
        if (!current_bfile.empty() && (current_bfile.compare(load_bfile) == 0)) {
            if (rampB.mode == rampB.DOWN) rampB.mode = rampB.UP;
            return;
        }
        rampB.mode = rampB.DOWN;
        gx_system::atomic_set(&ready, 0);
        sync();
        delete modelb;
        modelb = nullptr;
        mbSampleRate = 0;
        need_bresample = 0;
        clear_state_f();
        try {
            get_samplerate(std::string(load_bfile), &mbSampleRate);
            std::ifstream jsonStream(std::string(load_bfile), std::ifstream::binary);
            modelb = RTNeural::json_parser::parseJson<float>(jsonStream).release();
        } catch (const std::exception&) {
            gx_print_info("RTNeural Amp Modeler", "fail to load " + load_bfile);
            load_bfile = "None";
        }
        
        if (modelb) {
            current_bfile = load_bfile;
            modelb->reset();
            if (mbSampleRate <= 0) mbSampleRate = 48000;
            if (mbSampleRate > fSampleRate) {
                smpb.setup(fSampleRate, mbSampleRate);
                need_bresample = 1;
            } else if (mbSampleRate < fSampleRate) {
                smpb.setup(mbSampleRate, fSampleRate);
                need_bresample = 2;
            } 
             //fprintf(stderr, "B: %s\n", load_bfile.c_str());
        } 
        gx_system::atomic_set(&ready, 1);
    }
    rampB.mode = rampB.UP;
}

// non rt callback
void RtNeuralMulti::create_rtneural_afilelist() {
    if (load_apath.empty()) return;
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(load_apath);
    rtneural_afile_names.clear();
    int i = 0;
    if (file->query_exists()) {
        Glib::RefPtr<Gio::FileEnumerator> child_enumeration =
              file->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_NAME
                        "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME
                        "," G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
        Glib::RefPtr<Gio::FileInfo> file_info;
        rtneural_afile_names.push_back("None");
        while ((file_info = child_enumeration->next_file())) {
            std::string name = file_info->get_name();
            if ((name.compare(std::max<int>(0, name.size()-4), 4, "idax") == 0) ||
                (name.compare(std::max<int>(0, name.size()-4), 4, "json") == 0)) {
                rtneural_afile_names.push_back(file_info->get_name());
                i++;
                if ( i > 126) break;
            }
        }
    }
    for (;i<127;i++) rtneural_afile_names.push_back("None");
}

// non rt callback
void RtNeuralMulti::create_rtneural_bfilelist() {
    if (load_bpath.empty()) return;
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(load_bpath);
    rtneural_bfile_names.clear();
    int i = 0;
    if (file->query_exists()) {
        Glib::RefPtr<Gio::FileEnumerator> child_enumeration =
              file->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_NAME
                        "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME
                        "," G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
        Glib::RefPtr<Gio::FileInfo> file_info;
        rtneural_bfile_names.push_back("None");
        while ((file_info = child_enumeration->next_file())) {
            std::string name = file_info->get_name();
            if ((name.compare(std::max<int>(0, name.size()-4), 4, "idax") == 0) ||
                (name.compare(std::max<int>(0, name.size()-4), 4, "json") == 0)) {
                rtneural_bfile_names.push_back(file_info->get_name());
                i++;
                if ( i > 126) break;
            }
        }
    }
    for (;i<127;i++) rtneural_bfile_names.push_back("None");
}

int RtNeuralMulti::register_par(const ParamReg& reg)
{
    reg.registerFloatVar((idstring + ".input").c_str(),N_("Input A"),"S",N_("gain (dB)"),&fVslider0, 0.0, -20.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".inputb").c_str(),N_("Input B"),"S",N_("gain (dB)"),&fVslider01, 0.0, -20.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".cdelay").c_str(),N_("Delta Delay"),"S",N_("Delay A/B"),&fVslider02, 0.0, -4096.0, 4096.0, 1.0, 0);
    reg.registerFloatVar((idstring + ".output").c_str(),N_("Output"),"S",N_("gain (dB)"),&fVslider1, 0.0, -20.0, 20.0, 0.1, 0);
    reg.registerFloatVar((idstring + ".mix").c_str(),N_("Mix"),"S",N_("mix models"),&fVslider2, 0.5, 0.0, 1.0, 0.01, 0);
    param.reg_string((idstring + ".loadapath").c_str(), "", &load_apath, "", true)->set_desc(N_("load path for A *.json files"));
    param.reg_string((idstring + ".loadbpath").c_str(), "", &load_bpath, "", true)->set_desc(N_("load path for B *.json files"));
    param.reg_string((idstring + ".loadafile").c_str(), "", &load_afile, "*.json", true)->set_desc(N_("import *.json file"));
    param.reg_string((idstring + ".loadbfile").c_str(), "", &load_bfile, "*.json", true)->set_desc(N_("import *.json file"));
    reg.registerFloatVar((idstring + ".falist").c_str(),N_("select json/aidax File"),"S",N_("Select json/aidax file"),&afilelist, 0, 0, 127, 1, 0);
    reg.registerFloatVar((idstring + ".fblist").c_str(),N_("select json/aidax File"),"S",N_("Select json/aidax file"),&bfilelist, 0, 0, 127, 1, 0);

    param[(idstring + ".loadapath").c_str()].signal_changed_string().connect(
        sigc::hide(sigc::mem_fun(this, &RtNeuralMulti::create_rtneural_afilelist)));
    param[(idstring + ".loadbpath").c_str()].signal_changed_string().connect(
        sigc::hide(sigc::mem_fun(this, &RtNeuralMulti::create_rtneural_bfilelist)));
    param[(idstring + ".falist").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &RtNeuralMulti::load_json_afile_impl)));
    param[(idstring + ".fblist").c_str()].signal_changed_float().connect(
        sigc::hide(sigc::mem_fun(this, &RtNeuralMulti::load_json_bfile_impl)));


//    param[(idstring + ".loadafile").c_str()].signal_changed_string().connect(
//        sigc::hide(sigc::mem_fun(this, &RtNeuralMulti::load_json_afile)));
//    param[(idstring + ".loadbfile").c_str()].signal_changed_string().connect(
//        sigc::hide(sigc::mem_fun(this, &RtNeuralMulti::load_json_bfile)));
    return 0;
}

int RtNeuralMulti::register_params_static(const ParamReg& reg)
{
    return static_cast<RtNeuralMulti*>(reg.plugin)->register_par(reg);
}

inline int RtNeuralMulti::load_ui_f(const UiBuilder& b, int form)
{
    if (form & UI_FORM_GLADE) {
        b.load_glade_file((idstring + "_ui.glade").c_str());
        return 0;
    }
    if (form & UI_FORM_STACK) {

        b.openHorizontalhideBox("");
            b.create_master_slider((idstring + "output").c_str(), "Output");
        b.closeBox();
        b.openHorizontalBox("");
            b.openVerticalBox("");
                b.create_mid_rackknob((idstring + ".input").c_str(), "Input A");
                b.create_mid_rackknob((idstring + ".inputb").c_str(), "Input B");
            b.closeBox();
            b.openVerticalBox("");
                b.create_fload_switch(sw_button, nullptr, (idstring + ".loadafile").c_str());
                b.create_fload_switch(sw_button, nullptr, (idstring + ".loadbfile").c_str());
            b.closeBox();
            b.create_mid_rackknob((idstring + ".output").c_str(), "Output");
            b.create_mid_rackknob((idstring + ".mix").c_str(), "Mix");

        b.closeBox();

        return 0;
    }
    return -1;
}

int RtNeuralMulti::load_ui_f_static(const UiBuilder& b, int form)
{
    return static_cast<RtNeuralMulti*>(b.plugin)->load_ui_f(b, form);
}

void RtNeuralMulti::del_instance(PluginDef *p)
{
    delete static_cast<RtNeuralMulti*>(p);
}

} // namespace gx_engine
