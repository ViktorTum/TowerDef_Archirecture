#include <string>

#include <catch2/catch_test_macros.hpp>

#include "td/core/Version.h"

using namespace td::core;

TEST_CASE("Version parses and prints major.minor.patch", "[version]") {
    const auto r = Version::parse("1.2.3");
    REQUIRE(r.isOk());
    CHECK(r.value().major() == 1);
    CHECK(r.value().minor() == 2);
    CHECK(r.value().patch() == 3);
    CHECK(r.value().toString() == "1.2.3");
    CHECK(Version::parse("0.0.0").isOk());
    CHECK(Version::parse("4294967295.0.0").isOk());
}

TEST_CASE("Version rejects anything but three plain numbers", "[version]") {
    for (const char* bad : {"", "1", "1.2", "1.2.3.4", "1.2.x", "01.2.3", "-1.2.3", "1..3", ".1.2",
                            "1.2.", " 1.2.3", "1.2.3 ", "4294967296.0.0", "99999999999.0.0"}) {
        INFO("[" << bad << "]");
        const auto r = Version::parse(bad);
        REQUIRE_FALSE(r.isOk());
        CHECK(r.error().code() == "invalidVersion");
    }
}

TEST_CASE("Version ordering compares numbers, not text", "[version]") {
    const Version a(1, 2, 3);
    CHECK(a == Version(1, 2, 3));
    CHECK(a != Version(1, 2, 4));
    CHECK(a < Version(1, 10, 0));
    CHECK(a < Version(2, 0, 0));
    CHECK(Version(1, 2, 3) < Version(1, 2, 4));
    CHECK_FALSE(a < a);
}
