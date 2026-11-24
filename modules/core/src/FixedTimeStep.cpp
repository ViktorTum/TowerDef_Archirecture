#include "td/core/FixedTimeStep.h"

#include <algorithm>

namespace td::core {

Result<FixedTimeStep> FixedTimeStep::create(Scalar dtSeconds) {
    if (!NumericUtils::isFinite(dtSeconds) || !(dtSeconds > 0))
        return Result<FixedTimeStep>::failure(
            Diagnostic::error("outOfRange", "", "time step must be finite and greater than 0"));
    return Result<FixedTimeStep>::success(FixedTimeStep(dtSeconds));
}

Result<Tick> FixedTimeStep::delayToTicks(Scalar seconds) const {
    if (!NumericUtils::isFinite(seconds) || seconds < 0)
        return Result<Tick>::failure(
            Diagnostic::error("outOfRange", "", "delay must be finite and not negative"));
    return Result<Tick>::success(Tick(NumericUtils::ceilTolerant(seconds / dtSeconds_)));
}

Result<Tick> FixedTimeStep::intervalToTicks(Scalar seconds) const {
    if (!NumericUtils::isFinite(seconds) || !(seconds > 0))
        return Result<Tick>::failure(
            Diagnostic::error("outOfRange", "", "interval must be finite and greater than 0"));
    const std::int64_t ticks = NumericUtils::ceilTolerant(seconds / dtSeconds_);
    return Result<Tick>::success(Tick(std::max<std::int64_t>(1, ticks)));
}

Scalar FixedTimeStep::ticksToSeconds(Tick ticks) const {
    return static_cast<Scalar>(ticks.value()) * dtSeconds_;
}

}  // namespace td::core
