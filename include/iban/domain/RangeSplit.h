/// @file RangeSplit.h
/// @brief Divides a range into equal contiguous parts, one per thread: no gaps,
///        no overlap, and part sizes that differ by at most one.
// LEARN: Domain layer: pure arithmetic. It splits into "parts" and does not know
// LEARN: that the caller will give each part to a thread.

#pragma once

#include <vector>

#include "iban/domain/Range.h"

namespace iban::domain {

/// @brief Splits @p range into exactly @p parts contiguous parts.
///
/// With n values and p parts, every part gets n / p values and the first
/// n % p parts get one extra, so sizes differ by at most one. Part i ends where
/// part i + 1 begins, so together the parts cover the range without gaps or
/// overlap. When p > n, the last parts are empty.
/// Contiguous blocks are used instead of interleaving (x, x + p, x + 2p, ...)
/// so that each thread works on consecutive numbers.
/// @param range The range to split; may be empty.
/// @param parts Number of parts; must be at least 1.
/// @return Exactly @p parts ranges in ascending order. O(p) time.
/// @throws std::invalid_argument when @p parts < 1 or range.end < range.begin.
// LEARN: Same rule as the m-check: invalid arguments are programming errors,
// LEARN: so they throw; the parser already guarantees valid input from users.
[[nodiscard]] std::vector<Range> splitRange(Range range, int parts);

}  // namespace iban::domain
