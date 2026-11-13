/// @file ParallelCount.cpp
/// @brief Implementation of the parallel count. See ParallelCount.h.

#include "iban/application/ParallelCount.h"

#include <cstddef>
#include <numeric>
#include <thread>
#include <vector>

#include "iban/domain/Count.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"

namespace iban::application {

namespace {

// Waits for every thread that is still running.
// LEARN: A std::thread that is destroyed while still joinable calls
// LEARN: std::terminate(), so every started thread must be joined (or
// detached).
void joinAll(std::vector<std::thread> &threads) {
  for (std::thread &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

} // namespace

std::int64_t countParallel(const Query &query) {
  const std::vector<domain::Range> parts = domain::splitRange(
      domain::Range{query.begin, query.end}, query.threadCount);

  // One result slot per thread: thread i writes only counts[i]. Different
  // elements are different memory locations, so there is no data race.
  std::vector<std::int64_t> counts(parts.size(), 0);

  // std::thread with explicit joins, as in the lectures, so every step is
  // visible. std::jthread (C++20) would do this automatically: it joins in its
  // destructor (RAII), so no thread is left unjoined when an exception occurs,
  // and it offers a std::stop_token to ask a thread to stop early.
  // LEARN: With std::jthread the try/catch and both joinAll calls disappear:
  // LEARN: the vector's destructor joins every thread, also during an
  // exception. LEARN: The stop_token is useful for search mode, where threads
  // must stop LEARN: as soon as one of them has found the number.
  std::vector<std::thread> threads;
  // LEARN: reserve first, so emplace_back never moves running thread objects.
  threads.reserve(parts.size());
  try {
    for (std::size_t index = 0; index < parts.size(); ++index) {
      // LEARN: Capture by reference: parts and counts outlive the threads,
      // LEARN: because every thread is joined before this function returns.
      // LEARN: index and modulus are copied, so each thread has its own.
      threads.emplace_back([&parts, &counts, index, modulus = query.modulus] {
        // An exception cannot leave a thread: it would end the program.
        // countValid cannot throw here, because the query is validated.
        counts[index] = domain::countValid(parts[index], modulus);
      });
    }
  } catch (...) {
    // Starting a thread can fail (std::system_error, for example when the
    // system runs out of threads). The threads already running use parts
    // and counts, so they are joined before the error is passed on.
    joinAll(threads);
    throw;
  }
  joinAll(threads);

  // After join, every write of a thread is visible here, so reading the slots
  // needs no further synchronisation.
  // LEARN: std::accumulate adds all elements; the start value sets the type.
  return std::accumulate(counts.begin(), counts.end(), std::int64_t{0});
}

} // namespace iban::application
