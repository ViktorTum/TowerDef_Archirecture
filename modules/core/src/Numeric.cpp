#include "td/core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace td::core {

namespace {

bool nearInteger(Scalar value, Scalar relEps, Scalar& nearest) {
    nearest = std::round(value);
    return std::fabs(value - nearest) <= relEps * std::max(1.0, std::fabs(nearest));
}

void requireFinite(Scalar value) {
    if (!std::isfinite(value)) throw std::invalid_argument("value must be finite");
}

}  // namespace

bool NumericUtils::isFinite(Scalar value) { return std::isfinite(value); }

bool NumericUtils::inClosedRange(Scalar value, Scalar lo, Scalar hi) {
    return std::isfinite(value) && lo <= value && value <= hi;
}

std::int64_t NumericUtils::ceilTolerant(Scalar value, Scalar relEps) {
    requireFinite(value);
    Scalar nearest = 0;
    if (nearInteger(value, relEps, nearest)) return static_cast<std::int64_t>(nearest);
    return static_cast<std::int64_t>(std::ceil(value));
}

std::int64_t NumericUtils::floorTolerant(Scalar value, Scalar relEps) {
    requireFinite(value);
    Scalar nearest = 0;
    if (nearInteger(value, relEps, nearest)) return static_cast<std::int64_t>(nearest);
    return static_cast<std::int64_t>(std::floor(value));
}

bool NumericUtils::approxEqual(Scalar a, Scalar b, Scalar relEps) {
    if (!std::isfinite(a) || !std::isfinite(b)) return false;
    return std::fabs(a - b) <= relEps * std::max({1.0, std::fabs(a), std::fabs(b)});
}

}  // namespace td::core
