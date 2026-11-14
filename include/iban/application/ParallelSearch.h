/// @file ParallelSearch.h
/// @brief Search mode (u = 2) with p threads: finds the account number s in
///        [b, e) and stops the other threads as soon as it is found.
// LEARN: Application layer, like ParallelCount and ParallelList: the domain
// LEARN: searches one part, this layer adds the threads and the stop flag.

#pragma once

#include <cstdint>

#include "iban/application/Query.h"

namespace iban::application {

/// @brief Searches [query.begin, query.end) for the account number s
///        (query.target), using exactly query.threadCount threads.
///
/// Every thread searches the valid numbers of its own part. The thread that
/// finds s sets a shared atomic stop flag; the other threads look at the flag
/// before every number and stop (searchWithStopFlag in SearchVariants.h). The
/// meaning of s is described in docs/decisions/0013-meaning-of-s.md.
/// @param query A validated query in search mode, so with a target s.
/// @return s when it lies in the range and passes the m-check; otherwise, -1.
/// @throws std::invalid_argument when the query has no target.
/// @throws std::system_error when a thread cannot be started; threads that
///         were already started are joined first.
[[nodiscard]] std::int64_t searchParallel(const Query& query);

}  // namespace iban::application
