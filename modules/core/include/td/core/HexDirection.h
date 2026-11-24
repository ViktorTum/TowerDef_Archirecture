#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace td::core {

// The six neighbours of a hex. The order below is fixed: path search always
// visits neighbours in this order, so equal-length routes are always broken
// the same way on every machine.
enum class HexDirection : std::uint8_t { D0, D1, D2, D3, D4, D5 };

struct HexOffset {
    int dq;
    int dr;
};

inline constexpr std::array<HexDirection, 6> allDirections() {
    return {HexDirection::D0, HexDirection::D1, HexDirection::D2,
            HexDirection::D3, HexDirection::D4, HexDirection::D5};
}

// Axial offsets: (+1,0) (+1,-1) (0,-1) (-1,0) (-1,+1) (0,+1)
inline constexpr HexOffset offset(HexDirection d) {
    constexpr HexOffset table[6] = {{1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}};
    return table[static_cast<std::size_t>(d)];
}

}  // namespace td::core
