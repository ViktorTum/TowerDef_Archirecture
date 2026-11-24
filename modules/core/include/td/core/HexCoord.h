#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>

#include "td/core/HexDirection.h"

namespace td::core {

// Axial coordinates (q, r) of a hex cell: the discrete part of the world,
// "which cell". Pure integer arithmetic, independent of Scalar.
class HexCoord {
public:
    HexCoord() : q_(0), r_(0) {}
    HexCoord(std::int32_t q, std::int32_t r) : q_(q), r_(r) {}

    std::int32_t q() const { return q_; }
    std::int32_t r() const { return r_; }

    HexCoord neighbor(HexDirection d) const {
        const HexOffset o = offset(d);
        return {q_ + o.dq, r_ + o.dr};
    }

    // In the canonical direction order.
    std::array<HexCoord, 6> neighbors() const {
        std::array<HexCoord, 6> out;
        std::size_t i = 0;
        for (HexDirection d : allDirections()) out[i++] = neighbor(d);
        return out;
    }

    std::int32_t distanceTo(HexCoord other) const {
        const std::int32_t dq = other.q_ - q_;
        const std::int32_t dr = other.r_ - r_;
        return (std::abs(dq) + std::abs(dr) + std::abs(dq + dr)) / 2;
    }

    friend bool operator==(HexCoord a, HexCoord b) { return a.q_ == b.q_ && a.r_ == b.r_; }
    friend bool operator!=(HexCoord a, HexCoord b) { return !(a == b); }
    friend bool operator<(HexCoord a, HexCoord b) {
        return a.q_ != b.q_ ? a.q_ < b.q_ : a.r_ < b.r_;
    }

private:
    std::int32_t q_;
    std::int32_t r_;
};

}  // namespace td::core

namespace std {
template <>
struct hash<td::core::HexCoord> {
    std::size_t operator()(td::core::HexCoord c) const noexcept {
        const auto q = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.q()));
        const auto r = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.r()));
        return std::hash<std::uint64_t>{}((q << 32) | r);
    }
};
}  // namespace std
