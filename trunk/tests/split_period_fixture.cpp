#include "gx_split_period.h"
#include <cassert>
#include <limits>

int main() {
    gx_jack::SplitPeriod period;
    const float prior[] = {0.125f, -0.25f, 0.5f};
    const float next[] = {0.375f, 0.625f, -0.875f};
    // Uneven period length, including the JACK frame counter rollover.
    period.resize(3);
    const uint32_t start = std::numeric_limits<uint32_t>::max() - 1;
    period.put(start, prior, 3);
    period.put(start + 3, next, 3);
    for (unsigned i = 0; i < 3; ++i) {
        assert(period.get(start + i) == prior[i]);
        assert(period.get(start + 3 + i) == next[i]);
    }
    assert(period.get(start + 6) == 0.0f);
    period.put(start + 8, next, 3);
    for (unsigned i = 0; i < 3; ++i) {
        assert(period.get(start + i) == 0.0f);
        assert(period.get(start + 8 + i) == next[i]);
    }
    period.resize(32);
    for (unsigned i = 0; i < 3; ++i)
        assert(period.get(start + 8 + i) == 0.0f);
}
