/// @file FindValid.h
/// @brief Searches a range for the first number that passes the m-check and a
///        given condition, on the calling thread, and stops early when asked.
///        Search mode runs it on every thread.
// LEARN: A thread cannot be stopped from outside in C++. It has to ask itself,
// LEARN: regularly, whether it should stop: that is the stopRequested callback.

#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>

#include "iban/domain/MCheck.h"
#include "iban/domain/Range.h"

namespace iban::domain {

/// @brief Returns the first x in @p range, in ascending order, with
///        passesMCheck(x, modulus) and matches(x).
///
/// stopRequested() is called before every number; when it returns true, the
/// search ends without a result, because another thread has already found one.
/// @tparam Matches Callable as matches(std::int64_t) -> bool: the condition
///         for "found".
/// @tparam StopRequested Callable as stopRequested() -> bool.
/// @param range Half-open range of non-negative numbers; may be empty.
/// @param modulus Must be > 0.
/// @return The number found, or std::nullopt when no number in the range
///         matches or the search was stopped.
/// @throws std::invalid_argument when @p modulus <= 0 or the range is negative.
// LEARN: The condition is a parameter, so the search does not know what s means.
// LEARN: Another meaning of s (decision 0013) only changes the matches callable.
template <typename Matches, typename StopRequested>
[[nodiscard]] std::optional<std::int64_t> findValid(Range range, int modulus, Matches&& matches,
                                                    StopRequested&& stopRequested) {
    // Checked once, as in countValid, so an empty range still reports it.
    if (modulus <= 0) {
        throw std::invalid_argument("modulus must be greater than 0");
    }
    for (std::int64_t number = range.begin; number < range.end; ++number) {
        if (stopRequested()) {
            return std::nullopt;
        }
        // The search walks the valid numbers and treats the condition as a
        // black box, so the m-check comes first.
        if (passesMCheck(number, modulus) && matches(number)) {
            return number;
        }
    }
    return std::nullopt;
}

}  // namespace iban::domain
