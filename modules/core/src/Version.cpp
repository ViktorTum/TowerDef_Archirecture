#include "td/core/Version.h"

#include <array>
#include <cstddef>

namespace td::core {

namespace {

Result<Version> bad(std::string_view text) {
    return Result<Version>::failure(Diagnostic::error(
        "invalidVersion", "", "version \"" + std::string(text) + "\" must look like 1.2.3"));
}

// A non-empty run of digits without a leading zero, fitting into uint32.
bool parsePart(std::string_view part, std::uint32_t& out) {
    if (part.empty() || part.size() > 10) return false;
    if (part.size() > 1 && part[0] == '0') return false;
    std::uint64_t value = 0;
    for (char c : part) {
        if (c < '0' || c > '9') return false;
        value = value * 10 + static_cast<std::uint64_t>(c - '0');
    }
    if (value > 0xFFFFFFFFULL) return false;
    out = static_cast<std::uint32_t>(value);
    return true;
}

}  // namespace

Result<Version> Version::parse(std::string_view text) {
    std::array<std::uint32_t, 3> parts{};
    std::size_t begin = 0;
    for (std::size_t i = 0; i < 3; ++i) {
        const std::size_t end = (i < 2) ? text.find('.', begin) : text.size();
        if (end == std::string_view::npos) return bad(text);
        if (!parsePart(text.substr(begin, end - begin), parts[i])) return bad(text);
        begin = end + 1;
    }
    return Result<Version>::success(Version(parts[0], parts[1], parts[2]));
}

std::string Version::toString() const {
    return std::to_string(major_) + "." + std::to_string(minor_) + "." + std::to_string(patch_);
}

}  // namespace td::core
