/// @file Count.h
/// @brief Counts the numbers in a range that pass the m-check, on the calling
///        thread. Used by every thread for its own part, and on its own as the
///        sequential reference for tests and measurements.
// LEARN: Domain layer: no threads here. Parallelism is added one layer up, by
// LEARN: calling this function once per part on a separate thread.

#pragma once

#include <cstdint>

#include "iban/domain/Range.h"

namespace iban::domain {

/// @brief Number of values x in @p range for which passesMCheck(x, modulus).
/// @param range Half-open range of non-negative numbers; may be empty.
/// @param modulus Must be > 0.
/// @return The count, between 0 and range.size(). O(n * d) for n values of d digits.
/// @throws std::invalid_argument when @p modulus <= 0 or the range is negative.
// LEARN: Thread-safe: reads only its arguments and local variables, so several
// LEARN: threads may call it at the same time on different parts.
[[nodiscard]] std::int64_t countValid(Range range, int modulus);

}  // namespace iban::domain
