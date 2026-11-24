#pragma once
#include <cmath>

#include "td/core/Numeric.h"

namespace td::core {

// A point or a vector in the continuous plane of the map. One unit of length
// is one tile (the distance between centres of neighbouring hexes).
class Vec2 {
public:
    Vec2() : x_(0), y_(0) {}
    Vec2(Scalar x, Scalar y) : x_(x), y_(y) {}

    Scalar x() const { return x_; }
    Scalar y() const { return y_; }

    Vec2 operator+(Vec2 o) const { return {x_ + o.x_, y_ + o.y_}; }
    Vec2 operator-(Vec2 o) const { return {x_ - o.x_, y_ - o.y_}; }
    Vec2 operator*(Scalar k) const { return {x_ * k, y_ * k}; }
    Vec2 operator/(Scalar k) const { return {x_ / k, y_ / k}; }

    Scalar dot(Vec2 o) const { return x_ * o.x_ + y_ * o.y_; }
    Scalar lengthSquared() const { return dot(*this); }
    Scalar length() const { return std::sqrt(lengthSquared()); }
    Scalar distanceTo(Vec2 o) const { return (*this - o).length(); }

    // The zero vector stays the zero vector.
    Vec2 normalized() const {
        const Scalar len = length();
        return len == 0 ? Vec2() : *this / len;
    }

    // One step towards target that never overshoots it.
    Vec2 moveTowards(Vec2 target, Scalar maxStep) const {
        if (!(maxStep > 0)) return *this;
        const Vec2 delta = target - *this;
        const Scalar dist = delta.length();
        if (dist <= maxStep) return target;
        return *this + delta * (maxStep / dist);
    }

    // Exact comparison on purpose: "almost equal" is never used in the simulation.
    friend bool operator==(Vec2 a, Vec2 b) { return a.x_ == b.x_ && a.y_ == b.y_; }
    friend bool operator!=(Vec2 a, Vec2 b) { return !(a == b); }

private:
    Scalar x_;
    Scalar y_;
};

}  // namespace td::core
