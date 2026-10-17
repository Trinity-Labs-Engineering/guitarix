#include "gx_snapshot.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <new>
#include <thread>

static unsigned int allocations = 0;
void* operator new(std::size_t n) { ++allocations; if (void* p = std::malloc(n)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

using namespace gx_engine;
struct Dsp { float value = 1; unsigned int updates = 0; uint32_t applied = 0; };
static uint32_t callback_frame;
static void apply(void* target, double value) {
    auto& dsp = *static_cast<Dsp*>(target);
    dsp.value = value; ++dsp.updates; dsp.applied = callback_frame;
}

static void render(SnapshotScheduler& scheduler, uint32_t frame, Dsp& mono, Dsp& stereo,
                   float* left, float* right, bool stereo_first = false) {
    for (unsigned int i = 0; i < 2; ++i) {
        const unsigned int stream = stereo_first ? 1 - i : i;
        callback_frame = frame;
        scheduler.begin(stream, frame);
        if (stream == 1) {
            for (unsigned int sample = 0; sample < 32; ++sample)
                left[sample] = right[sample] = stereo.value;
            scheduler.finish(1, frame, 32, left, right);
        } else scheduler.finish(0, frame, 32);
    }
    (void)mono;
}

int main() {
    SnapshotScheduler a, b;
    Dsp am, as, bm, bs;
    std::vector<std::string> ah, bh;
    // Five settings cover all 20 non-self directed changes repeatedly. This
    // isolates the RT invariant; real legacy/split asset groups stay separate.
    for (unsigned int i = 0; i < 5; ++i) {
        const double value = i & 1 ? -0.7 : 0.7;
        ah.push_back(a.prepare({{&am, apply, value, 0}, {&as, apply, value, 1}}, true, {}).handle);
        bh.push_back(b.prepare({{&bm, apply, value, 0}, {&bs, apply, value, 1}}, true, {}).handle);
    }
    unsigned int transitions = 0;
    float left[32], right[32], other_l[32], other_r[32];
    for (unsigned int repeat = 0; repeat < 50; ++repeat) {
        for (unsigned int from = 0; from < 5; ++from) {
            for (unsigned int to = 0; to < 5; ++to) {
                if (from == to) continue;
                am.value = as.value = bm.value = bs.value = from & 1 ? -0.7f : 0.7f;
                const uint32_t start = transitions++ * 1024;
                const uint32_t target = start + 320;
                const auto before = allocations;
                assert(a.schedule(*a.find(ah[to]), target, start, 32, 192));
                assert(b.schedule(*b.find(bh[to]), target, start, 32, 192));
                float previous = as.value;
                float max_step = 0;
                unsigned int envelope_samples = 0;
                for (uint32_t frame = start; frame < start + 544; frame += 32) {
                    render(a, frame, am, as, left, right);
                    render(b, frame, bm, bs, other_l, other_r, true);
                    for (unsigned int sample = 0; sample < 32; ++sample) {
                        assert(left[sample] == other_l[sample]);
                        max_step = std::max(max_step, std::abs(left[sample] - previous));
                        previous = left[sample];
                        if (a.gain(frame + sample) != 1) ++envelope_samples;
                    }
                }
                assert(a.finished() && b.finished());
                assert(am.applied == target && as.applied == target);
                assert(bm.applied == target && bs.applied == target);
                assert(a.completion_frame() == b.completion_frame());
                assert(max_step < 0.008f); // bounded DC discontinuity, including sign reversal
                assert(envelope_samples == 191); // 4 ms at 48 kHz, zero only at cutover
                assert(allocations == before); // no heap work on schedule/audio path
                a.release(); b.release();
            }
        }
    }
    assert(transitions == 1000);
    // Both sides of signed/unsigned JACK wrap remain exact, including crossing
    // frame zero. Frames outside the scheduling margin are never applied late.
    auto* snap = a.find(ah[0]);
    assert(a.schedule(*snap, 64, 0xffffff00u, 32, 192));
    for (uint32_t f = 0xffffff00u; SnapshotScheduler::distance(f, 192) < 0; f += 32)
        render(a, f, am, as, left, right);
    assert(a.finished() && am.applied == 64 && as.applied == 64);
    a.release();
    assert(!a.schedule(*snap, 128, 0, 32, 192));
    assert(!a.schedule(*snap, 321, 0, 32, 192));
    const unsigned int old_updates = am.updates;
    assert(a.schedule(*snap, 320, 0, 32, 192));
    a.begin(0, 352);
    assert(a.failed() && am.updates == old_updates);
    a.release();
    assert(a.instance() != b.instance());
    const auto epoch = a.runtime_epoch();
    a.invalidate_runtime();
    assert(a.runtime_epoch() == epoch + 1);
    // Structural/asset changes make old handles unusable; snapshot controls
    // cannot silently retain pointers from a previous engine generation.
    a.invalidate();
    assert(!a.find(ah[0]));
    assert(!a.schedule(*snap, 320, 0, 32, 192));
    Dsp constant;
    auto& guarded = a.prepare({}, false, {{&constant,
        [](void* p, double v) { return static_cast<Dsp*>(p)->value == v; }, 1}});
    constant.value = 2;
    assert(!a.schedule(guarded, 320, 0, 32, 192));
    // Full prepared scenes write only their actual delta; the unchanged
    // secondary process truthfully reports no RT bypass change.
    Dsp unchanged;
    auto& no_delta = a.prepare({{&unchanged, apply, 1, 0,
        [](void* p) { return static_cast<double>(static_cast<Dsp*>(p)->value); }, true}}, true, {});
    assert(a.schedule(no_delta, 320, 0, 32, 192));
    for (uint32_t frame = 0; frame < 448; frame += 32)
        render(a, frame, am, as, left, right);
    assert(a.finished() && unchanged.updates == 0);
    assert(a.applied_control_count() == 0 && !a.rt_state_changed());
    a.release();
    unchanged.value = 0;
    assert(a.schedule(no_delta, 320, 0, 32, 192));
    for (uint32_t frame = 0; frame < 448; frame += 32)
        render(a, frame, am, as, left, right);
    assert(unchanged.updates == 1 && a.applied_control_count() == 1 && a.rt_state_changed());
    a.release();
    // A timeout must not let the control thread reclaim a snapshot while an
    // audio callback still owns its resolved control vector.
    std::atomic<bool> entered(false), unblock(false);
    struct Blocking { std::atomic<bool>* entered; std::atomic<bool>* unblock; } blocking{&entered, &unblock};
    auto& held = a.prepare({{&blocking, [](void* p, double) {
        auto& state = *static_cast<Blocking*>(p);
        state.entered->store(true);
        while (!state.unblock->load()) std::this_thread::yield();
    }, 0, 0}}, false, {});
    assert(a.schedule(held, 320, 0, 32, 192));
    std::thread callback([&] { a.begin(0, 320); });
    while (!entered.load()) std::this_thread::yield();
    a.release();
    assert(a.busy() && !a.quiescent());
    assert(!a.schedule(held, 640, 320, 32, 192));
    unblock.store(true);
    callback.join();
    assert(a.quiescent() && !a.busy());
    std::cout << "snapshot-ok: 1000 transitions, exact dual-client frames, 4ms envelope, no RT allocation\n";
}
