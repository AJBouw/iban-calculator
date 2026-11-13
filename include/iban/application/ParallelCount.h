/// @file ParallelCount.h
/// @brief Count mode (u = 0) with p threads: the range [b, e) is split fairly
///        and every thread counts its own part.
// LEARN: Application layer: combines the domain (splitRange, countValid) with
// LEARN: threads. The domain itself stays free of threading.

#pragma once

#include <cstdint>

#include "iban/application/Query.h"

namespace iban::application {

/// @brief Counts the valid numbers in [query.begin, query.end) using exactly
///        query.threadCount threads.
///
/// Every thread writes its result to its own slot, so no data is shared while
/// the threads run and no lock is needed; the slots are added after all
/// threads have been joined. A thread whose part is empty finishes at once.
/// @param query A validated query; the mode is not used.
/// @return The number of values that pass the m-check with query.modulus.
/// @throws std::system_error when a thread cannot be started; threads that
///         were already started are joined first.
[[nodiscard]] std::int64_t countParallel(const Query& query);

}  // namespace iban::application
