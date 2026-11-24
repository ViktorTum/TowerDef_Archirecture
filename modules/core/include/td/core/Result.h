#pragma once
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "td/core/Diagnostic.h"

namespace td::core {

// A value or a Diagnostic. Bad input from a file or a player is an expected
// situation, so it is returned, not thrown. Exceptions are kept for contract
// violations: calling value() on a failed Result is a programming error.
template <class T>
class Result {
    static_assert(!std::is_same_v<T, Diagnostic>, "Result<Diagnostic> would be ambiguous");

public:
    static Result success(T value) { return Result(std::in_place_index<0>, std::move(value)); }
    static Result failure(Diagnostic error) {
        return Result(std::in_place_index<1>, std::move(error));
    }

    bool isOk() const { return storage_.index() == 0; }

    const T& value() const {
        if (!isOk()) throw std::logic_error("Result::value() called on a failed result");
        return std::get<0>(storage_);
    }

    const Diagnostic& error() const {
        if (isOk()) throw std::logic_error("Result::error() called on a successful result");
        return std::get<1>(storage_);
    }

    T valueOr(T fallback) const { return isOk() ? std::get<0>(storage_) : std::move(fallback); }

private:
    template <std::size_t I, class U>
    Result(std::in_place_index_t<I> tag, U&& v) : storage_(tag, std::forward<U>(v)) {}

    std::variant<T, Diagnostic> storage_;
};

template <>
class Result<void> {
public:
    static Result success() { return Result(); }
    static Result failure(Diagnostic error) { return Result(std::move(error)); }

    bool isOk() const { return !error_.has_value(); }

    const Diagnostic& error() const {
        if (isOk()) throw std::logic_error("Result::error() called on a successful result");
        return *error_;
    }

private:
    Result() = default;
    explicit Result(Diagnostic error) : error_(std::move(error)) {}

    std::optional<Diagnostic> error_;
};

}  // namespace td::core
