#include <stdexcept>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "td/core/ConfigId.h"
#include "td/core/DiagnosticList.h"
#include "td/core/Registry.h"
#include "td/core/Result.h"

using namespace td::core;

namespace {
MechanicId mech(const char* text) { return MechanicId::tryParse(text).value(); }
}  // namespace

TEST_CASE("Diagnostic keeps severity, code, path and message", "[diagnostics]") {
    const auto e = Diagnostic::error("outOfRange", "waves[2].groups[0].count", "must be > 0");
    CHECK(e.severity() == Severity::Error);
    CHECK(e.code() == "outOfRange");
    CHECK(e.fieldPath() == "waves[2].groups[0].count");
    CHECK(e.message() == "must be > 0");
    CHECK(Diagnostic::warning("unknownField", "x", "ignored").severity() == Severity::Warning);
}

TEST_CASE("DiagnosticList collects everything in order", "[diagnostics]") {
    DiagnosticList list;
    CHECK(list.empty());
    CHECK_FALSE(list.hasErrors());

    list.add(Diagnostic::error("a", "p1", "m1"));
    list.add(Diagnostic::warning("b", "p2", "m2"));
    list.add(Diagnostic::error("c", "p3", "m3"));

    CHECK(list.size() == 3);
    CHECK(list.hasErrors());
    CHECK(list.errors().size() == 2);
    CHECK(list.warnings().size() == 1);

    std::vector<std::string> codes;
    for (const Diagnostic& d : list) codes.push_back(d.code());
    CHECK(codes == std::vector<std::string>{"a", "b", "c"});
}

TEST_CASE("warnings alone do not count as errors", "[diagnostics]") {
    DiagnosticList list;
    list.add(Diagnostic::warning("w", "", "just a note"));
    CHECK_FALSE(list.hasErrors());
    CHECK_FALSE(list.empty());
}

TEST_CASE("DiagnosticList merge appends", "[diagnostics]") {
    DiagnosticList a;
    DiagnosticList b;
    a.add(Diagnostic::error("x", "", ""));
    b.add(Diagnostic::error("y", "", ""));
    b.add(Diagnostic::warning("z", "", ""));
    a.merge(b);
    CHECK(a.size() == 3);
    CHECK(b.size() == 2);
}

TEST_CASE("Result carries a value or an error", "[result]") {
    const auto ok = Result<int>::success(42);
    CHECK(ok.isOk());
    CHECK(ok.value() == 42);
    CHECK(ok.valueOr(7) == 42);

    const auto bad = Result<int>::failure(Diagnostic::error("boom", "f", "broken"));
    CHECK_FALSE(bad.isOk());
    CHECK(bad.error().code() == "boom");
    CHECK(bad.valueOr(7) == 7);
}

TEST_CASE("asking a Result for the wrong side is a contract violation", "[result]") {
    const auto ok = Result<int>::success(1);
    const auto bad = Result<int>::failure(Diagnostic::error("e", "", ""));
    CHECK_THROWS_AS(bad.value(), std::logic_error);
    CHECK_THROWS_AS(ok.error(), std::logic_error);
}

TEST_CASE("Result works with non-trivial types", "[result]") {
    auto r = Result<std::string>::success(std::string("hello"));
    REQUIRE(r.isOk());
    CHECK(r.value() == "hello");
    CHECK(Result<std::string>::failure(Diagnostic::error("e", "", "")).valueOr("fallback") ==
          "fallback");
}

TEST_CASE("Result<void> is success or an error", "[result]") {
    CHECK(Result<void>::success().isOk());
    const auto bad = Result<void>::failure(Diagnostic::error("e", "p", "m"));
    CHECK_FALSE(bad.isOk());
    CHECK(bad.error().fieldPath() == "p");
    CHECK_THROWS_AS(Result<void>::success().error(), std::logic_error);
}

TEST_CASE("Registry add, find and contains", "[registry]") {
    Registry<MechanicId, int> reg;
    CHECK(reg.add(mech("slow"), 1).isOk());
    CHECK(reg.add(mech("burn"), 2).isOk());

    CHECK(reg.size() == 2);
    CHECK(reg.contains(mech("slow")));
    CHECK_FALSE(reg.contains(mech("poison")));
    REQUIRE(reg.find(mech("burn")) != nullptr);
    CHECK(*reg.find(mech("burn")) == 2);
    CHECK(reg.find(mech("poison")) == nullptr);
}

TEST_CASE("Registry refuses a duplicate id and keeps the first value", "[registry]") {
    Registry<MechanicId, int> reg;
    REQUIRE(reg.add(mech("slow"), 1).isOk());
    const auto again = reg.add(mech("slow"), 99);
    REQUIRE_FALSE(again.isOk());
    CHECK(again.error().code() == "duplicateId");
    CHECK(*reg.find(mech("slow")) == 1);
    CHECK(reg.size() == 1);
}

TEST_CASE("Registry replace and remove", "[registry]") {
    Registry<MechanicId, int> reg;
    reg.add(mech("slow"), 1);

    CHECK(reg.replace(mech("slow"), 5).isOk());
    CHECK(*reg.find(mech("slow")) == 5);

    const auto missing = reg.replace(mech("burn"), 5);
    REQUIRE_FALSE(missing.isOk());
    CHECK(missing.error().code() == "unknownId");

    CHECK(reg.remove(mech("slow")));
    CHECK_FALSE(reg.remove(mech("slow")));
    CHECK(reg.size() == 0);
}

TEST_CASE("Registry walks in key order, whatever the insertion order", "[registry]") {
    Registry<MechanicId, int> reg;
    reg.add(mech("c"), 3);
    reg.add(mech("a"), 1);
    reg.add(mech("b"), 2);

    const auto ids = reg.ids();
    REQUIRE(ids.size() == 3);
    CHECK(ids[0].str() == "a");
    CHECK(ids[1].str() == "b");
    CHECK(ids[2].str() == "c");

    std::vector<int> values;
    for (const auto& item : reg) values.push_back(item.second);
    CHECK(values == std::vector<int>{1, 2, 3});
}
