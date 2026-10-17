/* Prepared same-matrix snapshots. No allocation, locks, parameter signals or
 * plugin lifecycle operations occur in the JACK callbacks. */
#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace gx_engine {

struct SnapshotControl {
    void* target;
    void (*apply)(void*, double);
    double value;
    unsigned int stream; // 0 mono, 1 stereo (including separate JACK clients)
    double (*read)(void*) = nullptr;
    bool bypass = false;
};

struct SnapshotGuard {
    void* target;
    bool (*matches)(void*, double);
    double value;
};

struct PreparedSnapshot {
    std::string handle;
    unsigned int generation;
    std::vector<SnapshotControl> controls;
    bool bypass_changed;
    std::vector<SnapshotGuard> guards;
    std::string matrix_handle;
    unsigned int pool_generation = 0;
};

class SnapshotScheduler {
    const std::string instance_id;
    std::map<std::string, PreparedSnapshot> prepared;
    std::atomic<PreparedSnapshot*> pending;
    std::atomic<unsigned int> matrix_generation;
    std::atomic<unsigned int> runtime_generation;
    std::atomic<unsigned int> readers;
    struct Reader {
        std::atomic<unsigned int>& count;
        explicit Reader(std::atomic<unsigned int>& c): count(c) { count.fetch_add(1); }
        ~Reader() { count.fetch_sub(1); }
    };
    unsigned int serial;
    uint32_t target_frame;
    unsigned int half_fade;
    std::atomic<int> progress[2]; // 0 waiting, 1 applied, 2 output finished, -1 late/stale
    std::atomic<uint32_t> completed_frame[2];
    std::atomic<uint32_t> first_output_frame;
    std::atomic<unsigned int> applied_controls;
    std::atomic<bool> changed_bypass;
public:
    SnapshotScheduler(): instance_id(std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
        ":" + std::to_string(reinterpret_cast<std::uintptr_t>(this))),
        pending(nullptr), matrix_generation(1), runtime_generation(1), readers(0), serial(0),
        target_frame(0), half_fade(1), first_output_frame(0), applied_controls(0), changed_bypass(false) {
        for (unsigned int i = 0; i < 2; ++i) {
            progress[i].store(0); completed_frame[i].store(0);
        }
    }
    static int32_t distance(uint32_t a, uint32_t b) {
        return static_cast<int32_t>(a - b); // JACK's wrapping 32-bit frame clock
    }
    unsigned int generation() const { return matrix_generation.load(); }
    const std::string& instance() const { return instance_id; }
    void invalidate() { matrix_generation.fetch_add(1); }
    void invalidate_runtime() { runtime_generation.fetch_add(1); invalidate(); }
    unsigned int runtime_epoch() const { return runtime_generation.load(); }
    bool quiescent() const { return readers.load() == 0; }
    bool busy() const { return pending.load() != nullptr || !quiescent(); }
    PreparedSnapshot& prepare(std::vector<SnapshotControl> controls, bool bypass,
                              std::vector<SnapshotGuard> guards) {
        // Stale objects are reclaimed only off the audio thread, with no
        // scheduled callback holding a pointer into this map.
        for (auto i = prepared.begin(); i != prepared.end();) {
            if (i->second.generation != generation()) i = prepared.erase(i);
            else ++i;
        }
        if (prepared.size() >= 256) prepared.erase(prepared.begin());
        const std::string handle = instance_id + ":" + std::to_string(generation()) + ":" + std::to_string(++serial);
        PreparedSnapshot snapshot{handle, generation(), std::move(controls), bypass, std::move(guards), {}, 0};
        return prepared.emplace(handle, std::move(snapshot)).first->second;
    }
    PreparedSnapshot* find(const std::string& handle) {
        auto i = prepared.find(handle);
        return i != prepared.end() && i->second.generation == generation() ? &i->second : nullptr;
    }
    bool schedule(PreparedSnapshot& snapshot, uint32_t target, uint32_t now,
                  unsigned int block, unsigned int fade_samples) {
        if (busy() || snapshot.generation != generation() || !block ||
            target % block || fade_samples < 2 ||
            distance(target, now) <= static_cast<int32_t>(fade_samples / 2 + block)) return false;
        for (const auto& guard : snapshot.guards) {
            if (!guard.matches(guard.target, guard.value)) return false;
        }
        first_output_frame.store(0);
        applied_controls.store(0);
        changed_bypass.store(false);
        target_frame = target;
        half_fade = fade_samples / 2;
        for (unsigned int i = 0; i < 2; ++i) {
            progress[i].store(0); completed_frame[i].store(0);
        }
        pending.store(&snapshot, std::memory_order_release);
        return true;
    }
    void begin(unsigned int stream, uint32_t frame) {
        if (!pending.load()) return;
        Reader reader(readers);
        PreparedSnapshot* snapshot = pending.load();
        if (!snapshot || progress[stream].load(std::memory_order_relaxed) != 0) return;
        const int32_t offset = distance(frame, target_frame);
        if (snapshot->generation != generation() || offset > 0) {
            progress[stream].store(-1, std::memory_order_release);
        } else if (offset == 0) {
            for (const auto& control : snapshot->controls) {
                if (control.stream != stream ||
                    (control.read && control.read(control.target) == control.value)) continue;
                control.apply(control.target, control.value);
                applied_controls.fetch_add(1, std::memory_order_relaxed);
                if (control.bypass) changed_bypass.store(true, std::memory_order_relaxed);
            }
            progress[stream].store(1, std::memory_order_release);
        }
    }
    float gain(uint32_t frame) const {
        const int32_t offset = distance(frame, target_frame);
        if (offset <= -static_cast<int32_t>(half_fade) ||
            offset >= static_cast<int32_t>(half_fade)) return 1.0f;
        return static_cast<float>(offset < 0 ? -offset : offset) / half_fade;
    }
    void finish(unsigned int stream, uint32_t frame, unsigned int count,
                float* left = nullptr, float* right = nullptr) {
        if (!pending.load()) return;
        Reader reader(readers);
        if (!pending.load()) return;
        // The output stage is the one global envelope. Fading the mono stage
        // as well would feed a changing signal into stateful downstream DSP.
        if (stream == 1 && left && right) {
            for (unsigned int i = 0; i < count; ++i) {
                const float level = gain(frame + i);
                left[i] *= level; right[i] *= level;
            }
        }
        if (stream == 1 && frame == target_frame && progress[stream].load() == 1)
            first_output_frame.store(frame + count);
        if (progress[stream].load(std::memory_order_relaxed) == 1 &&
            distance(frame + count, target_frame) >= static_cast<int32_t>(half_fade)) {
            completed_frame[stream].store(frame + count, std::memory_order_relaxed);
            progress[stream].store(2, std::memory_order_release);
        }
    }
    bool finished() const { return progress[0].load() == 2 && progress[1].load() == 2; }
    bool failed() const { return progress[0].load() < 0 || progress[1].load() < 0; }
    uint32_t completion_frame() const { return completed_frame[1].load(); }
    uint32_t first_completed_output_frame() const { return first_output_frame.load(); }
    unsigned int applied_control_count() const { return applied_controls.load(); }
    bool rt_state_changed() const { return changed_bypass.load(); }
    void release() { pending.store(nullptr); }
};
} // namespace gx_engine
