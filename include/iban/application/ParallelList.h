/// @file ParallelList.h
/// @brief List mode (u = 1) with p threads: every valid number in [b, e) is
///        written at once as "<sequence number> <number>".
// LEARN: Application layer, like ParallelCount: the domain finds the numbers,
// LEARN: this layer adds the threads and the shared output.

#pragma once

#include <cstdint>
#include <ostream>

#include "iban/application/Query.h"

namespace iban::application {

/// @brief Writes every valid number in [query.begin, query.end) to @p out as
///        one line "<sequence number> <number>", using exactly
///        query.threadCount threads.
///
/// A thread writes a number as soon as it finds it; the sequence number and
/// the line are written under one std::mutex, so the sequence numbers ascend in
/// the order of the lines (listWithLock in ListVariants.h). The order of the
/// account numbers themselves depends on the threads and may differ per run.
/// @param query A validated query; the mode is not used.
/// @param out Stream for the lines, normally std::cout.
/// @return The number of lines written.
/// @throws std::system_error when a thread cannot be started; threads that
///         were already started are joined first.
std::int64_t listParallel(const Query& query, std::ostream& out);

}  // namespace iban::application
