#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include "td/core/Result.h"

namespace td::core {

// major.minor.patch. The schema version of a file, the version of the
// simulation rules and the version of a plugin all share this one type.
// Compatibility rules ("is this schema supported") live in the blocks that
// load files and replays, not here.
class Version {
public:
    Version(std::uint32_t major, std::uint32_t minor, std::uint32_t patch)
        : major_(major), minor_(minor), patch_(patch) {}

    static Result<Version> parse(std::string_view text);  // "1.2.3", nothing else

    std::uint32_t major() const { return major_; }
    std::uint32_t minor() const { return minor_; }
    std::uint32_t patch() const { return patch_; }

    std::string toString() const;

    friend bool operator==(const Version& a, const Version& b) {
        return a.major_ == b.major_ && a.minor_ == b.minor_ && a.patch_ == b.patch_;
    }
    friend bool operator!=(const Version& a, const Version& b) { return !(a == b); }
    friend bool operator<(const Version& a, const Version& b) {
        if (a.major_ != b.major_) return a.major_ < b.major_;
        if (a.minor_ != b.minor_) return a.minor_ < b.minor_;
        return a.patch_ < b.patch_;
    }

private:
    std::uint32_t major_;
    std::uint32_t minor_;
    std::uint32_t patch_;
};

}  // namespace td::core
