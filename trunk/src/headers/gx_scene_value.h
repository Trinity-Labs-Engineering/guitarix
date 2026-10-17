/* Prepared scene scalar validation. GPL-2.0-or-later. */
#pragma once
#include <cmath>
#include <limits>

namespace gx_engine {

template<class Parameter>
bool valid_scene_scalar(const Parameter& parameter, double value) {
    // JSON numbers are doubles, but DSP controls and their bounds are floats.
    // Compare the value that the setter will actually store: e.g. JSON 0.1
    // must be accepted at a 0.1f lower bound, for both matrices and snapshots.
    if (!std::isfinite(value) ||
        (parameter.isFloat() && std::fabs(value) > std::numeric_limits<float>::max())) return false;
    const double stored = parameter.isFloat() ? static_cast<float>(value) : value;
    return std::isfinite(stored) &&
        (!parameter.isBool() || value == 0 || value == 1) &&
        (!parameter.isInt() || value == std::floor(value)) &&
        (!parameter.hasRange() ||
         (stored >= parameter.getLowerAsFloat() && stored <= parameter.getUpperAsFloat()));
}

} // namespace gx_engine
