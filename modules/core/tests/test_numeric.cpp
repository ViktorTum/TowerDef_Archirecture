#include <cmath>
#include <limits>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include "td/core/Numeric.h"

using namespace td::core;

TEST_CASE("isFinite rejects NaN and infinity", "[numeric]") {
    CHECK(NumericUtils::isFinite(1.5));
    CHECK(NumericUtils::isFinite(0.0));
    CHECK_FALSE(NumericUtils::isFinite(std::numeric_limits<Scalar>::infinity()));
    CHECK_FALSE(NumericUtils::isFinite(std::nan("")));
}

TEST_CASE("inClosedRange includes both ends", "[numeric]") {
    CHECK(NumericUtils::inClosedRange(0.0, 0.0, 1.0));
    CHECK(NumericUtils::inClosedRange(1.0, 0.0, 1.0));
    CHECK(NumericUtils::inClosedRange(0.5, 0.0, 1.0));
    CHECK_FALSE(NumericUtils::inClosedRange(1.0001, 0.0, 1.0));
    CHECK_FALSE(NumericUtils::inClosedRange(-0.0001, 0.0, 1.0));
    CHECK_FALSE(NumericUtils::inClosedRange(std::nan(""), 0.0, 1.0));
}

TEST_CASE("plain ceil is wrong for 0.14 / 0.02, ceilTolerant is right", "[numeric]") {
    const Scalar ratio = 0.14 / 0.02;  // 7.000000000000001
    CHECK(std::ceil(ratio) == 8.0);
    CHECK(NumericUtils::ceilTolerant(ratio) == 7);

    const Scalar triple = 0.1 * 3 / 0.1;  // 3.0000000000000004
    CHECK(std::ceil(triple) == 4.0);
    CHECK(NumericUtils::ceilTolerant(triple) == 3);
}

TEST_CASE("ceilTolerant still rounds real fractions up", "[numeric]") {
    CHECK(NumericUtils::ceilTolerant(0.2) == 1);
    CHECK(NumericUtils::ceilTolerant(2.3) == 3);
    CHECK(NumericUtils::ceilTolerant(7.0) == 7);
    CHECK(NumericUtils::ceilTolerant(0.0) == 0);
    CHECK(NumericUtils::ceilTolerant(-0.5) == 0);
}

TEST_CASE("floorTolerant: 0.29 * 100 is 29, not 28", "[numeric]") {
    const Scalar value = 0.29 * 100;  // 28.999999999999996
    CHECK(std::floor(value) == 28.0);
    CHECK(NumericUtils::floorTolerant(value) == 29);
    CHECK(NumericUtils::floorTolerant(2.7) == 2);
    CHECK(NumericUtils::floorTolerant(-0.5) == -1);
}

TEST_CASE("approxEqual uses a relative tolerance", "[numeric]") {
    CHECK(NumericUtils::approxEqual(0.1 + 0.2, 0.3));
    CHECK(NumericUtils::approxEqual(1e9, 1e9 + 0.5));
    CHECK_FALSE(NumericUtils::approxEqual(1.0, 1.1));
    CHECK_FALSE(NumericUtils::approxEqual(std::nan(""), std::nan("")));
}

TEST_CASE("rounding a non-finite value is a contract violation", "[numeric]") {
    CHECK_THROWS_AS(NumericUtils::ceilTolerant(std::nan("")), std::invalid_argument);
    CHECK_THROWS_AS(NumericUtils::floorTolerant(std::numeric_limits<Scalar>::infinity()),
                    std::invalid_argument);
}
