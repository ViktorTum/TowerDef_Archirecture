#include <cmath>
#include <limits>
#include <stdexcept>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "td/core/FixedTimeStep.h"
#include "td/core/Tick.h"

using namespace td::core;
using Catch::Approx;

TEST_CASE("Tick basics", "[time]") {
    CHECK(Tick().value() == 0);
    CHECK(Tick(412).value() == 412);
    CHECK(Tick(5).next() == Tick(6));
    CHECK(Tick(5) + Tick(3) == Tick(8));
    CHECK(Tick(5) - Tick(3) == Tick(2));
}

TEST_CASE("Tick comparisons", "[time]") {
    CHECK(Tick(1) < Tick(2));
    CHECK(Tick(2) <= Tick(2));
    CHECK(Tick(3) > Tick(2));
    CHECK(Tick(3) >= Tick(3));
    CHECK(Tick(3) != Tick(4));
}

TEST_CASE("Tick cannot be negative", "[time]") {
    CHECK_THROWS_AS(Tick(-1), std::invalid_argument);
    CHECK_THROWS_AS(Tick(2) - Tick(3), std::invalid_argument);
}

TEST_CASE("FixedTimeStep accepts only a finite positive step", "[time]") {
    CHECK(FixedTimeStep::create(0.05).isOk());
    for (Scalar bad : {0.0, -0.05, std::numeric_limits<Scalar>::infinity(), std::nan("")}) {
        const auto r = FixedTimeStep::create(bad);
        REQUIRE_FALSE(r.isOk());
        CHECK(r.error().code() == "outOfRange");
    }
}

TEST_CASE("delayToTicks may give zero and rounds up", "[time]") {
    const auto dt = FixedTimeStep::create(0.05).value();
    CHECK(dt.delayToTicks(0.0).value() == Tick(0));
    CHECK(dt.delayToTicks(0.01).value() == Tick(1));
    CHECK(dt.delayToTicks(1.0).value() == Tick(20));
}

TEST_CASE("0.14 s at dt 0.02 is 7 ticks, not 8", "[time]") {
    // 0.14 / 0.02 is 7.000000000000001 in double; a plain ceil would give 8.
    const auto dt = FixedTimeStep::create(0.02).value();
    CHECK(dt.delayToTicks(0.14).value() == Tick(7));
    CHECK(dt.intervalToTicks(0.14).value() == Tick(7));
}

TEST_CASE("0.75 s at dt 0.05 is exactly 15 ticks", "[time]") {
    const auto dt = FixedTimeStep::create(0.05).value();
    CHECK(dt.delayToTicks(0.75).value() == Tick(15));
}

TEST_CASE("delayToTicks rejects negative and non-finite input", "[time]") {
    const auto dt = FixedTimeStep::create(0.05).value();
    CHECK_FALSE(dt.delayToTicks(-0.1).isOk());
    CHECK_FALSE(dt.delayToTicks(std::nan("")).isOk());
    CHECK_FALSE(dt.delayToTicks(std::numeric_limits<Scalar>::infinity()).isOk());
}

TEST_CASE("intervalToTicks takes at least one tick", "[time]") {
    const auto dt = FixedTimeStep::create(0.05).value();
    CHECK(dt.intervalToTicks(0.001).value() == Tick(1));
    CHECK(dt.intervalToTicks(0.05).value() == Tick(1));
    CHECK(dt.intervalToTicks(0.06).value() == Tick(2));
}

TEST_CASE("intervalToTicks rejects zero and negative input", "[time]") {
    const auto dt = FixedTimeStep::create(0.05).value();
    CHECK_FALSE(dt.intervalToTicks(0.0).isOk());
    CHECK_FALSE(dt.intervalToTicks(-1.0).isOk());
}

TEST_CASE("ticksToSeconds converts back", "[time]") {
    const auto dt = FixedTimeStep::create(0.05).value();
    CHECK(dt.seconds() == 0.05);
    CHECK(dt.ticksToSeconds(Tick(20)) == Approx(1.0));
    CHECK(dt.ticksToSeconds(Tick(0)) == 0.0);
}
