#include <cassert>
#include <cmath>
#include <initializer_list>
#define __rt_data
#include "clipping.h"

int main() {
    assert(asymclip(0.) == 0. && asymclip(-0.) == 0.);
    for (double sign : {-1., 1.}) {
        double previous = 0;
        for (int i = 1; i < 100; ++i) {
            double x = sign * i * 1e-5;
            double y = asymclip(x);
            assert(std::signbit(y) != std::signbit(x));
            assert(std::fabs(y) > previous);
            previous = std::fabs(y);
        }
        const auto& table = *cliptable[sign < 0 ? 1 : 0];
        const double boundary = 3 * table.low / (1. - table.low);
        assert(std::fabs(asymclip(sign * (boundary - 1e-8)) -
                         asymclip(sign * (boundary + 1e-8))) < 1e-7);
        // The nonlinear portion of the existing curve is unchanged.
        const double index = 20.25;
        const double warped = table.low + index / table.istep;
        const double x = sign * 3 * warped / (1 - warped);
        const double expected = std::copysign(.75 * table.data[20] + .25 * table.data[21], -x);
        assert(std::fabs(asymclip(x) - expected) < 1e-7);
    }
}
