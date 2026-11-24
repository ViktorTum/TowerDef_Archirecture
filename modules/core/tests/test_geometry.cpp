#include <limits>
#include <stdexcept>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "td/core/Circle.h"
#include "td/core/Vec2.h"

using namespace td::core;
using Catch::Approx;

TEST_CASE("Vec2 arithmetic", "[geometry]") {
    const Vec2 a(1, 2);
    const Vec2 b(3, 5);
    CHECK(a + b == Vec2(4, 7));
    CHECK(b - a == Vec2(2, 3));
    CHECK(a * 2 == Vec2(2, 4));
    CHECK(b / 2 == Vec2(1.5, 2.5));
    CHECK(a != b);
    CHECK(Vec2() == Vec2(0, 0));
}

TEST_CASE("Vec2 dot, length and distance", "[geometry]") {
    CHECK(Vec2(1, 2).dot(Vec2(3, 4)) == 11.0);
    CHECK(Vec2(3, 4).lengthSquared() == 25.0);
    CHECK(Vec2(3, 4).length() == 5.0);
    CHECK(Vec2(1, 1).distanceTo(Vec2(4, 5)) == 5.0);
}

TEST_CASE("Vec2 normalized has length 1, the zero vector stays zero", "[geometry]") {
    CHECK(Vec2(0, 5).normalized() == Vec2(0, 1));
    CHECK(Vec2(3, 4).normalized().length() == Approx(1.0));
    CHECK(Vec2(0, 0).normalized() == Vec2(0, 0));
}

TEST_CASE("moveTowards never overshoots", "[geometry]") {
    const Vec2 from(0, 0);
    const Vec2 target(10, 0);
    CHECK(from.moveTowards(target, 3) == Vec2(3, 0));
    CHECK(from.moveTowards(target, 10) == target);
    CHECK(from.moveTowards(target, 99) == target);
    CHECK(from.moveTowards(target, 0) == from);
    CHECK(from.moveTowards(target, -1) == from);
    CHECK(target.moveTowards(target, 1) == target);
}

TEST_CASE("Circle contains and overlaps include the boundary", "[geometry]") {
    const Circle c(Vec2(0, 0), 2);
    CHECK(c.contains(Vec2(1, 1)));
    CHECK(c.contains(Vec2(2, 0)));
    CHECK_FALSE(c.contains(Vec2(2.01, 0)));

    CHECK(c.overlaps(Circle(Vec2(3, 0), 1)));
    CHECK_FALSE(c.overlaps(Circle(Vec2(3.01, 0), 1)));
    CHECK(c.overlaps(Circle(Vec2(0.1, 0), 0.1)));
}

TEST_CASE("Circle needs a finite positive radius", "[geometry]") {
    CHECK_THROWS_AS(Circle(Vec2(), 0), std::invalid_argument);
    CHECK_THROWS_AS(Circle(Vec2(), -1), std::invalid_argument);
    CHECK_THROWS_AS(Circle(Vec2(), std::numeric_limits<Scalar>::infinity()), std::invalid_argument);
}
