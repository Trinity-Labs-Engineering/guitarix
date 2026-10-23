#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace gx_engine {

// The callback owns history while idle. During a guarded scene replay it
// instead produces into pending; the control thread owns history and DSP.
// Publishing the consumed cursor AFTER rendering is the ownership handoff:
// the callback may resume only when every queued input has finished rendering.
class SceneInputHistory {
public:
    static constexpr unsigned int history_capacity = 98304; // 512 ms at 192 kHz
    static constexpr unsigned int pending_capacity = 65536;
    enum Mode { idle, pause_requested, paused, resume_requested, abort_requested };
private:
    std::array<float, history_capacity> history;
    std::array<float, pending_capacity> pending;
    unsigned int history_limit = 24576;
    unsigned int history_head = 0;
    unsigned int history_size = 0;
    std::atomic<unsigned int> produced{0};
    std::atomic<unsigned int> consumed{0};
    std::atomic<Mode> mode{idle};
    std::atomic<bool> overflow{false};
    std::atomic<bool> history_invalidated{false};

    void append_history(const float* input, unsigned int count) {
        if (count >= history_limit) {
            input += count - history_limit;
            count = history_limit;
        }
        const unsigned int first = (std::min)(count, history_limit - history_head);
        std::memcpy(history.data() + history_head, input, first * sizeof(float));
        std::memcpy(history.data(), input + first, (count - first) * sizeof(float));
        history_head = (history_head + count) % history_limit;
        history_size = (std::min)(history_limit, history_size + count);
    }
public:
    // Only called while the audio engine is stopped for a rate change.
    void set_samplerate(unsigned int rate) {
        history_limit = (std::max)(1u, (std::min)(history_capacity, rate * 512u / 1000u));
        history_head = history_size = 0;
    }
    void invalidate_rt() { history_head = history_size = 0; }
    void invalidate_control() { history_invalidated.store(true, std::memory_order_release); }
    Mode state() const { return mode.load(std::memory_order_acquire); }
    bool failed() const { return overflow.load(std::memory_order_acquire); }
    void request_pause() {
        produced.store(0, std::memory_order_relaxed);
        consumed.store(0, std::memory_order_relaxed);
        overflow.store(false, std::memory_order_relaxed);
        mode.store(pause_requested, std::memory_order_release);
    }
    void request_resume() { mode.store(resume_requested, std::memory_order_release); }
    void request_abort() { mode.store(abort_requested, std::memory_order_release); }
    void acknowledge_abort_rt() {
        if (mode.load(std::memory_order_acquire) != abort_requested) return;
        // A failed replay may have dropped unconsumed input. Do not splice
        // that discontinuous interval into a future scene's causal history.
        history_head = history_size = 0;
        produced.store(0, std::memory_order_relaxed);
        consumed.store(0, std::memory_order_relaxed);
        mode.store(idle, std::memory_order_release);
    }

    // Returns true while control owns the selected DSP; caller outputs silence.
    // This function allocates nothing, never waits, and never runs replay DSP.
    bool capture_rt(const float* input, unsigned int count) {
        acknowledge_abort_rt();
        Mode current = mode.load(std::memory_order_acquire);
        if ((current == idle || current == pause_requested) &&
            history_invalidated.exchange(false, std::memory_order_acq_rel))
            invalidate_rt();
        if (current == pause_requested) {
            mode.compare_exchange_strong(current, paused, std::memory_order_acq_rel);
            current = mode.load(std::memory_order_acquire);
        }
        if (current == abort_requested) {
            acknowledge_abort_rt();
            current = mode.load(std::memory_order_acquire);
        }
        if (current == resume_requested &&
             produced.load(std::memory_order_relaxed) == consumed.load(std::memory_order_acquire)) {
            // Control has stopped touching DSP/history before requesting an
            // abort, or finished every queued block before publishing consumed.
            mode.store(idle, std::memory_order_release);
            current = idle;
        }
        if (current == idle) {
            append_history(input, count);
            return false;
        }
        const unsigned int head = produced.load(std::memory_order_relaxed);
        const unsigned int tail = consumed.load(std::memory_order_acquire);
        if (count > pending_capacity || head - tail > pending_capacity - count) {
            overflow.store(true, std::memory_order_release);
            return true;
        }
        const unsigned int offset = head % pending_capacity;
        const unsigned int first = (std::min)(count, pending_capacity - offset);
        std::memcpy(pending.data() + offset, input, first * sizeof(float));
        std::memcpy(pending.data(), input + first, (count - first) * sizeof(float));
        produced.store(head + count, std::memory_order_release);
        return true;
    }

    // Control-only methods below require acknowledged pause ownership.
    unsigned int copy_history(float* output) {
        if (history_invalidated.exchange(false, std::memory_order_acq_rel))
            history_head = history_size = 0;
        const unsigned int start = (history_head + history_limit - history_size) % history_limit;
        const unsigned int first = (std::min)(history_size, history_limit - start);
        std::memcpy(output, history.data() + start, first * sizeof(float));
        std::memcpy(output + first, history.data(), (history_size - first) * sizeof(float));
        return history_size;
    }
    template<class Render>
    unsigned int consume(Render render) {
        const unsigned int tail = consumed.load(std::memory_order_relaxed);
        const unsigned int head = produced.load(std::memory_order_acquire);
        const unsigned int count = (std::min)(128u, head - tail);
        if (!count) return 0;
        float block[128];
        const unsigned int offset = tail % pending_capacity;
        const unsigned int first = (std::min)(count, pending_capacity - offset);
        std::memcpy(block, pending.data() + offset, first * sizeof(float));
        std::memcpy(block + first, pending.data(), (count - first) * sizeof(float));
        append_history(block, count);
        render(block, count);
        consumed.store(tail + count, std::memory_order_release);
        return count;
    }
};

} // namespace gx_engine
