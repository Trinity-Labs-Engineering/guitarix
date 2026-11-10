/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>

namespace gx_engine {
// Keep relay contact noise out of every downstream processor's history.
// Control requests are acknowledged only once the audio callback has reached
// the requested endpoint. The ordinary, open path is sample-identical.
class RouteInputGate {
    std::atomic<unsigned> requested{0}, acknowledged{0};
    unsigned generation = 0; // Serialized control thread only.
    float gain = 1.0f;
    float step = 1.0f / 96.0f;
public:
    static_assert(ATOMIC_INT_LOCK_FREE == 2, "route gate must be lock-free");
    void set_samplerate(unsigned rate) {
        step = 1.0f / std::max(1u, static_cast<unsigned>(std::ceil(rate * .002)));
    }
    unsigned request(bool hold) {
        generation += 2;
        const unsigned token = generation | static_cast<unsigned>(hold);
        requested.store(token, std::memory_order_release);
        return token;
    }
    bool ready(unsigned token) const {
        return acknowledged.load(std::memory_order_acquire) == token;
    }
    void process(float* samples, unsigned count, float* second = nullptr) {
        const unsigned token = requested.load(std::memory_order_acquire);
        const bool hold = (token & 1u) != 0;
        if (!hold && gain == 1.0f) {
            acknowledged.store(token, std::memory_order_release);
            return;
        }
        if (hold && gain == 0.0f) {
            std::memset(samples, 0, count * sizeof(float));
            if (second) std::memset(second, 0, count * sizeof(float));
            acknowledged.store(token, std::memory_order_release);
            return;
        }
        for (unsigned i = 0; i < count; ++i) {
            gain = hold ? std::max(0.0f, gain - step) : std::min(1.0f, gain + step);
            samples[i] *= gain;
            if (second) second[i] *= gain;
        }
        if (gain == (hold ? 0.0f : 1.0f))
            acknowledged.store(token, std::memory_order_release);
    }
};
}
