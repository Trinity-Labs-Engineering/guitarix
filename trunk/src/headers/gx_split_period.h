// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <memory>

namespace gx_jack {

// The FX callback publishes frame-tagged samples. The amp reads the previous
// period concurrently and treats any missing or overwritten frame as silence.
class SplitPeriod {
public:
    void resize(unsigned frames) {
        size_t capacity = 1;
        while (capacity < size_t(frames) * 2) capacity *= 2;
        history.reset(new std::atomic<uint64_t>[capacity]{});
        mask = capacity - 1;
    }

    void put(uint32_t start, const float* input, unsigned frames) {
        for (unsigned i = 0; i < frames; ++i) {
            uint32_t bits;
            std::memcpy(&bits, input + i, sizeof(bits));
            const uint32_t frame = start + i;
            history[frame & mask].store((uint64_t(frame) << 32) | bits, std::memory_order_relaxed);
        }
    }

    float get(uint32_t frame) const {
        const auto value = history[frame & mask].load(std::memory_order_relaxed);
        if (uint32_t(value >> 32) != frame) return 0.0f;
        const uint32_t bits = uint32_t(value);
        float sample;
        std::memcpy(&sample, &bits, sizeof(sample));
        return sample;
    }

private:
    static_assert(std::atomic<uint64_t>::is_always_lock_free, "split samples require lock-free publication");
    std::unique_ptr<std::atomic<uint64_t>[]> history;
    size_t mask = 0;
};
}
