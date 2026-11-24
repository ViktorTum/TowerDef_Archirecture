#pragma once
#include "td/core/Numeric.h"
#include "td/core/Result.h"
#include "td/core/Tick.h"

namespace td::core {

// The fixed simulation step dt (seconds). The only place that converts
// seconds to ticks and back.
class FixedTimeStep {
public:
    static Result<FixedTimeStep> create(Scalar dtSeconds);  // dt must be finite and > 0

    Scalar seconds() const { return dtSeconds_; }

    // A delay may be zero: ceil(seconds / dt), seconds >= 0.
    Result<Tick> delayToTicks(Scalar seconds) const;
    // An interval always takes at least one tick: max(1, ceil(seconds / dt)), seconds > 0.
    Result<Tick> intervalToTicks(Scalar seconds) const;

    Scalar ticksToSeconds(Tick ticks) const;

private:
    explicit FixedTimeStep(Scalar dtSeconds) : dtSeconds_(dtSeconds) {}

    Scalar dtSeconds_;
};

}  // namespace td::core
