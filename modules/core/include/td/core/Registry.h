#pragma once
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

#include "td/core/Result.h"

namespace td::core {

// Values by unique key, walked in a stable order. The base "dictionary with
// rules" for every registry in the system. std::map (ordered by key) is used
// instead of unordered_map because iteration order affects the simulation and
// serialisation, and it must be the same on every machine.
// Requirement on TId: operator< (ConfigId and HexCoord qualify).
template <class TId, class TValue>
class Registry {
public:
    using Container = std::map<TId, TValue>;

    Result<void> add(TId id, TValue value) {
        if (items_.count(id) != 0)
            return Result<void>::failure(
                Diagnostic::error("duplicateId", "", "this id is already registered"));
        items_.emplace(std::move(id), std::move(value));
        return Result<void>::success();
    }

    Result<void> replace(TId id, TValue value) {
        auto it = items_.find(id);
        if (it == items_.end())
            return Result<void>::failure(
                Diagnostic::error("unknownId", "", "there is nothing registered under this id"));
        it->second = std::move(value);
        return Result<void>::success();
    }

    bool remove(const TId& id) { return items_.erase(id) != 0; }

    const TValue* find(const TId& id) const {
        auto it = items_.find(id);
        return it == items_.end() ? nullptr : &it->second;
    }

    bool contains(const TId& id) const { return items_.count(id) != 0; }

    std::vector<TId> ids() const {
        std::vector<TId> out;
        out.reserve(items_.size());
        for (const auto& item : items_) out.push_back(item.first);
        return out;
    }

    std::size_t size() const { return items_.size(); }

    typename Container::const_iterator begin() const { return items_.begin(); }
    typename Container::const_iterator end() const { return items_.end(); }

private:
    Container items_;
};

}  // namespace td::core
