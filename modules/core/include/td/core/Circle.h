#pragma once
#include <stdexcept>

#include "td/core/Vec2.h"

namespace td::core {

// Centre + radius: an enemy's collision radius, a projectile's hit radius,
// the splash radius, a tower's range.
class Circle {
public:
    Circle(Vec2 center, Scalar radius) : center_(center), radius_(radius) {
        if (!NumericUtils::isFinite(radius) || !(radius > 0))
            throw std::invalid_argument("Circle radius must be finite and > 0");
    }

    Vec2 center() const { return center_; }
    Scalar radius() const { return radius_; }

    bool contains(Vec2 point) const {
        return (point - center_).lengthSquared() <= radius_ * radius_;
    }

    bool overlaps(const Circle& other) const {
        const Scalar reach = radius_ + other.radius_;
        return (other.center_ - center_).lengthSquared() <= reach * reach;
    }

private:
    Vec2 center_;
    Scalar radius_;
};

}  // namespace td::core
