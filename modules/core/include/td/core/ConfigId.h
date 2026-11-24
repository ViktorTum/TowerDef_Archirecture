#pragma once
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "td/core/Result.h"

namespace td::core {

// The name of a configuration entity ("fire_tower", "goblin", "level_1").
// Tag is an empty marker type: ConfigId<TowerTag> and ConfigId<EnemyTag> are
// different types, so mixing them up is a compile error, not a runtime bug.
// The only way to get one is tryParse, so an invalid id cannot exist.
template <class Tag>
class ConfigId {
public:
    static Result<ConfigId> tryParse(std::string_view text) {
        if (text.empty())
            return Result<ConfigId>::failure(
                Diagnostic::error("emptyId", "", "id must not be empty"));
        for (char c : text) {
            const bool allowed = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
            if (!allowed)
                return Result<ConfigId>::failure(Diagnostic::error(
                    "invalidId", "", "id \"" + std::string(text) + "\" must match [a-z0-9_]+"));
        }
        return Result<ConfigId>::success(ConfigId(std::string(text)));
    }

    const std::string& str() const { return value_; }

    friend bool operator==(const ConfigId& a, const ConfigId& b) { return a.value_ == b.value_; }
    friend bool operator!=(const ConfigId& a, const ConfigId& b) { return !(a == b); }
    friend bool operator<(const ConfigId& a, const ConfigId& b) { return a.value_ < b.value_; }

private:
    explicit ConfigId(std::string value) : value_(std::move(value)) {}

    std::string value_;
};

// The id of a mechanic: used by definitions (B) and by the mechanics block (E).
// Other domain ids (TowerTypeId, EnemyTypeId, ...) are declared next to their
// types in td::defs, each with its own tag.
struct MechanicTag {};
using MechanicId = ConfigId<MechanicTag>;

}  // namespace td::core

namespace std {
template <class Tag>
struct hash<td::core::ConfigId<Tag>> {
    std::size_t operator()(const td::core::ConfigId<Tag>& id) const noexcept {
        return std::hash<std::string>{}(id.str());
    }
};
}  // namespace std
