/// @file Range.h
/// @brief A half-open range of account numbers [begin, end), the unit of work
///        that is divided between the threads.
// LEARN: Half-open means begin is included and end is not: [10, 20) holds 10..19.
// LEARN: Adjacent ranges then share a boundary without overlap: [10, 20) [20, 30).

#pragma once

#include <cstdint>

namespace iban::domain {

/// A half-open range [begin, end). An empty range (begin == end) is valid and
/// occurs when there are more parts than numbers.
// LEARN: A struct with public fields: a range has no rules of its own beyond
// LEARN: begin <= end, which the code that creates ranges guarantees.
struct Range {
    std::int64_t begin = 0;  ///< first number in the range (inclusive)
    std::int64_t end = 0;    ///< one past the last number (exclusive)

    /// Number of values in the range.
    // LEARN: constexpr: can be evaluated at compile time, e.g. in static_assert.
    // LEARN: noexcept: promises not to throw, which lets callers rely on it.
    [[nodiscard]] constexpr std::int64_t size() const noexcept { return end - begin; }

    /// True when the range holds no values; a thread given an empty range
    /// finishes immediately.
    [[nodiscard]] constexpr bool empty() const noexcept { return begin == end; }

    // LEARN: = default lets the compiler compare all fields in order (C++20),
    // LEARN: so tests can write EXPECT_EQ(range, (Range{0, 5})).
    friend constexpr bool operator==(const Range&, const Range&) = default;
};

// Compile-time checks of the definition of a half-open range.
static_assert(Range{10, 20}.size() == 10, "end is exclusive");
static_assert(Range{5, 5}.empty(), "begin == end is empty");

}  // namespace iban::domain
