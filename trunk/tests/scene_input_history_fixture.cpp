#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "gx_scene_input_history.h"

using gx_engine::SceneInputHistory;

static std::vector<float> sequence(unsigned int first, unsigned int count) {
    std::vector<float> result(count);
    for (unsigned int index = 0; index < count; ++index) result[index] = float(first + index);
    return result;
}

static void chronological_history_and_pending() {
    SceneInputHistory history;
    history.set_samplerate(1000);
    unsigned int frame = 0;
    for (; frame < 800; frame += 32) {
        auto input = sequence(frame, 32);
        assert(!history.capture_rt(input.data(), input.size()));
    }
    history.request_pause();
    auto paused = sequence(frame, 32);
    assert(history.capture_rt(paused.data(), paused.size()));
    assert(history.state() == SceneInputHistory::paused);
    std::vector<float> copy(SceneInputHistory::history_capacity);
    unsigned int count = history.copy_history(copy.data());
    assert(count == 512);
    for (unsigned int index = 0; index < count; ++index) assert(copy[index] == float(288 + index));
    unsigned int rendered = 0;
    assert(history.consume([&](float* samples, unsigned int size) {
        for (unsigned int index = 0; index < size; ++index) assert(samples[index] == float(800 + index));
        rendered += size;
        // Real DSP is allowed to mutate its scratch input. Retained raw history
        // must already own the untouched samples for the next replay.
        std::fill(samples, samples + size, -1.0f);
    }) == 32);
    assert(rendered == 32);
    history.request_resume();
    auto next = sequence(832, 32);
    assert(!history.capture_rt(next.data(), next.size()));
    assert(history.state() == SceneInputHistory::idle);
    history.request_pause();
    next = sequence(864, 32);
    assert(history.capture_rt(next.data(), next.size()));
    count = history.copy_history(copy.data());
    assert(count == 512);
    for (unsigned int index = 0; index < count; ++index) assert(copy[index] == float(352 + index));
    history.request_abort();
    assert(!history.capture_rt(next.data(), next.size()));
}

static void queue_wrap_and_variable_blocks() {
    SceneInputHistory history;
    history.request_pause();
    unsigned int produced = 0, rendered = 0;
    for (unsigned int iteration = 0; iteration < 3000; ++iteration) {
        const unsigned int count = iteration % 3 == 0 ? 257 : 31;
        auto input = sequence(produced, count);
        assert(history.capture_rt(input.data(), input.size()));
        produced += count;
        while (history.consume([&](float* samples, unsigned int size) {
            assert(size <= 128);
            for (unsigned int index = 0; index < size; ++index) assert(samples[index] == float(rendered++));
        })) {}
        assert(produced == rendered);
        assert(!history.failed());
    }
    assert(produced > SceneInputHistory::pending_capacity * 4);
    history.request_resume();
    auto next = sequence(produced, 32);
    assert(!history.capture_rt(next.data(), next.size()));
}

static void overflow_can_abort_without_waiting() {
    SceneInputHistory history;
    history.request_pause();
    std::vector<float> full(SceneInputHistory::pending_capacity, 1.0f);
    assert(history.capture_rt(full.data(), full.size()));
    assert(!history.failed());
    assert(history.capture_rt(full.data(), 1));
    assert(history.failed());
    history.request_abort();
    assert(!history.capture_rt(full.data(), 32));
    assert(history.state() == SceneInputHistory::idle);
    history.request_pause();
    assert(!history.failed());
    assert(history.capture_rt(full.data(), 32));
    assert(history.consume([](float*, unsigned int count) { assert(count == 32); }) == 32);
}

static void rendering_owns_dsp_until_consumed_is_published() {
    SceneInputHistory history;
    history.request_pause();
    auto first = sequence(0, 128);
    assert(history.capture_rt(first.data(), first.size()));
    history.request_resume();
    std::atomic<bool> rendering{false}, finish_render{false};
    std::thread control([&] {
        assert(history.consume([&](float* samples, unsigned int count) {
            assert(count == 128);
            rendering.store(true, std::memory_order_release);
            while (!finish_render.load(std::memory_order_acquire)) std::this_thread::yield();
            for (unsigned int index = 0; index < count; ++index) assert(samples[index] == float(index));
        }) == 128);
    });
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!rendering.load(std::memory_order_acquire)) {
        assert(std::chrono::steady_clock::now() < deadline);
        std::this_thread::yield();
    }
    auto next = sequence(128, 32);
    // The only old pending block is already copied into control's scratch.
    // Publishing consumption before rendering would wrongly resume RT here.
    assert(history.capture_rt(next.data(), next.size()));
    assert(history.state() == SceneInputHistory::resume_requested);
    finish_render.store(true, std::memory_order_release);
    control.join();
    assert(history.consume([](float* samples, unsigned int count) {
        assert(count == 32);
        for (unsigned int index = 0; index < count; ++index) assert(samples[index] == float(128 + index));
    }) == 32);
    next = sequence(160, 32);
    assert(!history.capture_rt(next.data(), next.size()));
    assert(history.state() == SceneInputHistory::idle);
}

int main() {
    chronological_history_and_pending();
    queue_wrap_and_variable_blocks();
    overflow_can_abort_without_waiting();
    rendering_owns_dsp_until_consumed_is_published();
    std::cout << "scene input history ordering, overflow and DSP ownership pass\n";
}
