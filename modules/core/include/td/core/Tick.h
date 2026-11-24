#pragma once
#include <cstdint>
#include <stdexcept>

namespace td::core {

// A whole number of simulation ticks: either a point in time (tick 412) or a
// duration (a delay of 40 ticks). A separate type keeps ticks from being mixed
// with seconds or with plain integers.
class Tick {
public:
    explicit Tick(std::int64_t index = 0) : index_(index) {
        if (index < 0) throw std::invalid_argument("Tick must be >= 0");
    }

    std::int64_t value() const { return index_; }
    Tick next() const { return Tick(index_ + 1); }

    Tick operator+(Tick other) const { return Tick(index_ + other.index_); }
    Tick operator-(Tick other) const { return Tick(index_ - other.index_); }

    friend bool operator==(Tick a, Tick b) { return a.index_ == b.index_; }
    friend bool operator!=(Tick a, Tick b) { return a.index_ != b.index_; }
    friend bool operator<(Tick a, Tick b) { return a.index_ < b.index_; }
    friend bool operator<=(Tick a, Tick b) { return a.index_ <= b.index_; }
    friend bool operator>(Tick a, Tick b) { return a.index_ > b.index_; }
    friend bool operator>=(Tick a, Tick b) { return a.index_ >= b.index_; }

private:
    std::int64_t index_;
};

}  // namespace td::core
