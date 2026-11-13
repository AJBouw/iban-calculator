/// @file ParallelCount.h
/// @brief Count mode (u = 0) with p threads: the range [b, e) is split fairly,
///        every thread counts its own part and adds its count to a shared
///        counter.
// LEARN: Application layer: combines the domain (splitRange, countValid) with
// LEARN: threads. The domain itself stays free of threading.

#pragma once

#include <cstdint>

#include "iban/application/Query.h"

namespace iban::application {

/// @brief Counts the valid numbers in [query.begin, query.end) using exactly
///        query.threadCount threads.
///
/// The result is kept in a shared counter protected by a std::mutex, as the
/// assignment requires. Every thread first counts its own part locally and
/// then adds that count once under the lock, so the lock is taken only p times
/// (countSharedPerThread in CountVariants.h). A thread whose part is empty
/// finishes at once.
/// @param query A validated query; the mode is not used.
/// @return The number of values that pass the m-check with query.modulus.
/// @throws std::system_error when a thread cannot be started; threads that
///         were already started are joined first.
[[nodiscard]] std::int64_t countParallel(const Query& query);

}  // namespace iban::application
