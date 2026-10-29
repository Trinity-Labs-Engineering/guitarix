#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <map>
#include <new>
#include <string>
#include <vector>
#include "gx_plugin.h"
#include "gx_faust_support.h"

namespace effects {
#include "faust-generated/dattorros_progenitor.cc"
#include "faust-generated/duck_delay.cc"
#include "faust-generated/duck_delay_st.cc"
#include "faust-generated/panoram_enhancer.cc"
#include "faust-generated/spring_reverb.cc"
#include "faust-generated/plate_reverb.cc"
#include "faust-generated/hall_reverb.cc"
#include "faust-generated/ambient_reverb.cc"
#include "faust-generated/greyhole.cc"
#include "faust-generated/granular_cloud.cc"
}
namespace amps {
#include "faust-generated/gxamp6.cc"
#include "faust-generated/gxamp7.cc"
#include "faust-generated/gxamp8.cc"
#include "faust-generated/gxamp9.cc"
#include "faust-generated/gxamp11.cc"
#include "faust-generated/gxamp13.cc"
#include "faust-generated/gxamp16.cc"
#include "faust-generated/gxamp17.cc"
}
#undef min
#undef max

namespace pluginlib {
namespace mbdel { PluginDef* plugin(); }
namespace mbe { PluginDef* plugin(); }
namespace mbchor { PluginDef* plugin(); }
namespace zita_rev1 { PluginDef* plugin(); }
}

// Count actual large array allocations and inject partial-allocation failures.
// Scalars/metadata retain their ordinary allocation path.
struct alignas(std::max_align_t) ArrayAllocation { size_t bytes; };
static size_t live_history_bytes = 0, history_allocations = 0;
static int fail_after = -1;
void* operator new[](size_t bytes) {
    if (bytes > 40000 && fail_after >= 0 && fail_after-- == 0)
        throw std::bad_alloc();
    auto* allocation = static_cast<ArrayAllocation*>(std::malloc(sizeof(ArrayAllocation) + bytes));
    if (!allocation) throw std::bad_alloc();
    allocation->bytes = bytes;
    if (bytes > 40000) { live_history_bytes += bytes; ++history_allocations; }
    return allocation + 1;
}
void operator delete[](void* pointer) noexcept {
    if (!pointer) return;
    auto* allocation = static_cast<ArrayAllocation*>(pointer) - 1;
    if (allocation->bytes > 40000) live_history_bytes -= allocation->bytes;
    std::free(allocation);
}
void operator delete[](void* pointer, size_t) noexcept { ::operator delete[](pointer); }

static std::map<std::string, float*> controls;
static float* register_float(const char* id, const char*, const char*, const char*,
                             float* value, float initial, float, float, float, const value_pair*) {
    *value = initial; controls[id] = value; return value;
}
static int* register_int(const char*, const char*, const char*, const char*, int* value,
                         int initial, int, int, const value_pair*) {
    *value = initial; return value;
}
static bool* register_bool(const char*, const char*, const char*, const char*, bool* value, bool initial) {
    *value = initial; return value;
}

static std::vector<float> render(PluginDef* plugin) {
    float input[32], left[32], right[32];
    for (int i = 0; i < 32; ++i) input[i] = .1f * std::sin(i * .14f);
    if (plugin->mono_audio) plugin->mono_audio(32, input, left, plugin);
    else plugin->stereo_audio(32, input, input, left, right, plugin);
    for (float value : left) assert(std::isfinite(value));
    if (plugin->stereo_audio) for (float value : right) assert(std::isfinite(value));
    return {left, left + 32};
}

static void check(PluginDef* (*factory)(), bool resampled = false) {
    assert(live_history_bytes == 0);
    const size_t initial_allocations = history_allocations;
    auto* plugin = factory();
    controls.clear();
    ParamReg reg {plugin, register_float, register_int, register_bool};
    plugin->register_params(reg);
    const auto stable_controls = controls;
    plugin->set_samplerate(48000, plugin);
    plugin->clear_state(plugin); // Resetting a dormant module must be safe.
    assert(history_allocations == initial_allocations);
    assert(plugin->activate_plugin);

    // Both failure before allocation and failure part-way through a multiband
    // allocation must leave the processor safely retryable, with no leaked pages.
    for (int allocation = 0; allocation < 2; ++allocation) {
        fail_after = allocation;
        const int status = plugin->activate_plugin(true, plugin);
        fail_after = -1;
        if (!status) plugin->activate_plugin(false, plugin);
        assert(live_history_bytes == 0);
    }
    assert(plugin->activate_plugin(true, plugin) == 0);
    const size_t resident_bytes = live_history_bytes;
    const size_t prepared_allocations = history_allocations;
    assert(resident_bytes >= 262144);
    const auto clean = render(plugin);
    for (int i = 0; i < 8; ++i) render(plugin);
    assert(plugin->activate_plugin(true, plugin) == 0);
    plugin->clear_state(plugin); // Resident scene re-entry reuses DSP history storage.
    const auto reset = render(plugin);
    // Fixed-rate amps keep their resampler state on an ordinary DSP reset;
    // that is their pre-existing behavior and is distinct from delay history.
    if (!resampled)
        for (size_t i = 0; i < clean.size(); ++i) assert(std::abs(clean[i] - reset[i]) < 1e-6f);
    assert(history_allocations == prepared_allocations);
    assert(live_history_bytes == resident_bytes);

    // A rate notification for an active instance must reset its histories as
    // before, without allocating a second set of buffers.
    for (int i = 0; i < 8; ++i) render(plugin);
    plugin->set_samplerate(48000, plugin);
    const auto rate_reset = render(plugin);
    for (size_t i = 0; i < clean.size(); ++i) assert(std::abs(clean[i] - rate_reset[i]) < 1e-6f);
    assert(history_allocations == prepared_allocations);

    plugin->activate_plugin(false, plugin);
    assert(live_history_bytes == 0);
    plugin->set_samplerate(44100, plugin);
    assert(history_allocations == prepared_allocations);
    assert(controls == stable_controls);
    float* control = controls.begin()->second;
    const float saved_value = *control;
    assert(plugin->activate_plugin(true, plugin) == 0);
    assert(*control == saved_value);
    render(plugin);
    // Destruction is also valid without a separate deactivate call.
    plugin->delete_instance(plugin);
    assert(live_history_bytes == 0);
}

int main() {
    check(effects::dattorros_progenitor::plugin);
    check(effects::duck_delay::plugin);
    check(effects::duck_delay_st::plugin);
    check(effects::panoram_enhancer::plugin);
    check(effects::spring_reverb::plugin);
    check(effects::plate_reverb::plugin);
    check(effects::hall_reverb::plugin);
    check(effects::ambient_reverb::plugin);
    check(effects::greyhole::plugin);
    check(effects::granular_cloud::plugin);
    check(pluginlib::mbdel::plugin);
    check(pluginlib::mbe::plugin);
    check(pluginlib::mbchor::plugin);
    check(pluginlib::zita_rev1::plugin);
    check(amps::gxamp6::plugin, true);
    check(amps::gxamp7::plugin, true);
    check(amps::gxamp8::plugin, true);
    check(amps::gxamp9::plugin, true);
    check(amps::gxamp11::plugin, true);
    check(amps::gxamp13::plugin, true);
    check(amps::gxamp16::plugin, true);
    check(amps::gxamp17::plugin, true);
    std::cout << "lazy-dsp-ok\n";
}
