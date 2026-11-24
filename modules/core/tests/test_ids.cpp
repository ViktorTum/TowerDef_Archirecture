#include <string>
#include <type_traits>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "td/core/ConfigId.h"

using namespace td::core;

namespace {
struct TowerTag {};
struct EnemyTag {};
MechanicId mech(const char* text) { return MechanicId::tryParse(text).value(); }
}  // namespace

TEST_CASE("ConfigId accepts [a-z0-9_]+", "[ids]") {
    const auto r = MechanicId::tryParse("fire_tower_2");
    REQUIRE(r.isOk());
    CHECK(r.value().str() == "fire_tower_2");
    CHECK(MechanicId::tryParse("_").isOk());
    CHECK(MechanicId::tryParse("0").isOk());
}

TEST_CASE("ConfigId rejects empty text with a named code", "[ids]") {
    const auto r = MechanicId::tryParse("");
    REQUIRE_FALSE(r.isOk());
    CHECK(r.error().code() == "emptyId");
}

TEST_CASE("ConfigId rejects anything outside [a-z0-9_]", "[ids]") {
    for (const char* bad : {"Fire", "fire-tower", "fire tower", "fire.tower", "огонь", "a/b"}) {
        const auto r = MechanicId::tryParse(bad);
        INFO(bad);
        REQUIRE_FALSE(r.isOk());
        CHECK(r.error().code() == "invalidId");
    }
}

TEST_CASE("ConfigId of different tags are different types", "[ids]") {
    using TowerId = ConfigId<TowerTag>;
    using EnemyId = ConfigId<EnemyTag>;
    static_assert(!std::is_same_v<TowerId, EnemyId>);
    static_assert(!std::is_convertible_v<TowerId, EnemyId>);
    SUCCEED();
}

TEST_CASE("ConfigId cannot be built without validation", "[ids]") {
    static_assert(!std::is_constructible_v<MechanicId, std::string>);
    static_assert(!std::is_default_constructible_v<MechanicId>);
    SUCCEED();
}

TEST_CASE("ConfigId comparison and hashing", "[ids]") {
    CHECK(mech("a") == mech("a"));
    CHECK(mech("a") != mech("b"));
    CHECK(mech("a") < mech("b"));
    CHECK_FALSE(mech("b") < mech("a"));

    std::unordered_set<MechanicId> set;
    set.insert(mech("x"));
    set.insert(mech("x"));
    set.insert(mech("y"));
    CHECK(set.size() == 2);
}
