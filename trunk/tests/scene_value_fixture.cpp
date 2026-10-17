#include <cassert>
#include <initializer_list>
#include <limits>
#ifdef GX_TEST_FAUST_MACROS
#include "gx_faust_support.h"
#endif
#include "gx_scene_value.h"
#ifdef GX_TEST_FAUST_MACROS
static_assert(min(2, 3) == 2 && max(2, 3) == 3);
#endif

struct Parameter {
    char type;
    float lower, upper;
    bool ranged = true;
    bool isFloat() const { return type == 'f'; }
    bool isInt() const { return type == 'i'; }
    bool isBool() const { return type == 'b'; }
    bool hasRange() const { return ranged; }
    float getLowerAsFloat() const { return lower; }
    float getUpperAsFloat() const { return upper; }
};

int main() {
    using gx_engine::valid_scene_scalar;
    Parameter attack{'f', .1f, 500};
    assert(valid_scene_scalar(attack, 0.1));
    assert(valid_scene_scalar(attack, 500));
    assert(!valid_scene_scalar(attack, 0.099));
    assert(!valid_scene_scalar(attack, 500.01));
    Parameter upper{'f', -1, .7f};
    assert(valid_scene_scalar(upper, 0.7));
    assert(!valid_scene_scalar(upper, 0.701));
    for (double value : {std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::quiet_NaN(), 1e300}) {
        assert(!valid_scene_scalar(attack, value));
        assert(!valid_scene_scalar(Parameter{'f', 0, 0, false}, value));
    }
    Parameter selector{'i', 0, 5};
    assert(valid_scene_scalar(selector, 5));
    assert(!valid_scene_scalar(selector, 5.000000001));
    assert(!valid_scene_scalar(selector, 6));
    Parameter bypass{'b', 0, 1, false};
    assert(valid_scene_scalar(bypass, 0) && valid_scene_scalar(bypass, 1));
    assert(!valid_scene_scalar(bypass, 0.5) && !valid_scene_scalar(bypass, 2));
}
