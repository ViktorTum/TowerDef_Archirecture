#pragma once
#include <cstdint>

namespace td::core {

// The one place where the numeric type of the whole engine is chosen.
// Switching to fixed-point later means changing this alias and the classes
// of this module that use it (Vec2, Circle, ...).
using Scalar = double;

class NumericUtils {
public:
    NumericUtils() = delete;

    static bool isFinite(Scalar value);
    static bool inClosedRange(Scalar value, Scalar lo, Scalar hi);

    // Rounding that ignores floating-point noise: 0.14 / 0.02 is
    // 7.000000000000001, and ceilTolerant of it is 7, not 8.
    // Precondition: value is finite (std::invalid_argument otherwise).
    static std::int64_t ceilTolerant(Scalar value, Scalar relEps = 1e-9);
    static std::int64_t floorTolerant(Scalar value, Scalar relEps = 1e-9);

    static bool approxEqual(Scalar a, Scalar b, Scalar relEps = 1e-9);
};

}  // namespace td::core
