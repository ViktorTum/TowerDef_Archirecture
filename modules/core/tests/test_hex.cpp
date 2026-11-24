#include <set>
#include <stdexcept>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "td/core/HexCoord.h"
#include "td/core/HexDirection.h"

using namespace td::core;

TEST_CASE("HexDirection order and offsets are fixed", "[hex]") {
    const auto dirs = allDirections();
    REQUIRE(dirs.size() == 6);
    const HexOffset expected[6] = {{1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}};
    for (std::size_t i = 0; i < 6; ++i) {
        CHECK(offset(dirs[i]).dq == expected[i].dq);
        CHECK(offset(dirs[i]).dr == expected[i].dr);
    }
}

TEST_CASE("HexCoord neighbours follow the canonical order", "[hex]") {
    const HexCoord c(2, 3);
    const auto n = c.neighbors();
    CHECK(n[0] == HexCoord(3, 3));
    CHECK(n[1] == HexCoord(3, 2));
    CHECK(n[2] == HexCoord(2, 2));
    CHECK(n[3] == HexCoord(1, 3));
    CHECK(n[4] == HexCoord(1, 4));
    CHECK(n[5] == HexCoord(2, 4));
    CHECK(c.neighbor(HexDirection::D2) == n[2]);
}

TEST_CASE("every neighbour is at hex distance 1, opposite sides are distinct", "[hex]") {
    const HexCoord c(-4, 7);
    std::set<HexCoord> seen;
    for (const HexCoord& n : c.neighbors()) {
        CHECK(c.distanceTo(n) == 1);
        seen.insert(n);
    }
    CHECK(seen.size() == 6);
}

TEST_CASE("HexCoord distance", "[hex]") {
    CHECK(HexCoord(0, 0).distanceTo(HexCoord(0, 0)) == 0);
    CHECK(HexCoord(0, 0).distanceTo(HexCoord(2, -1)) == 2);
    CHECK(HexCoord(0, 0).distanceTo(HexCoord(3, -3)) == 3);
    CHECK(HexCoord(0, 0).distanceTo(HexCoord(2, 2)) == 4);
    CHECK(HexCoord(1, 1).distanceTo(HexCoord(-2, 0)) == HexCoord(-2, 0).distanceTo(HexCoord(1, 1)));
}

TEST_CASE("HexCoord ordering and hashing", "[hex]") {
    CHECK(HexCoord(1, 9) < HexCoord(2, 0));
    CHECK(HexCoord(1, 1) < HexCoord(1, 2));
    CHECK_FALSE(HexCoord(1, 2) < HexCoord(1, 2));
    CHECK(HexCoord(1, 2) != HexCoord(2, 1));

    std::unordered_set<HexCoord> set{{0, 0}, {0, 0}, {-1, 5}, {5, -1}};
    CHECK(set.size() == 3);
}
