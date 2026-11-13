/// @file CountVariants.h
/// @brief Ways to combine the counts of p threads, kept side by side so their
///        cost can be measured and compared. The program uses
///        countSharedPerThread<std::mutex> (see ParallelCount.h); the reasons
///        are in docs/decisions/0010-shared-counter-and-lock.md.
// LEARN: Every variant splits the range the same way and must give the same
// LEARN: count; they differ only in how the threads combine their results.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "iban/application/Query.h"
#include "iban/domain/Count.h"
#include "iban/domain/MCheck.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"
#include "iban/infrastructure/BasicLockable.h"
#include "iban/infrastructure/ForkJoin.h"
#include "iban/infrastructure/SharedCounter.h"

namespace iban::application {

/// @brief One result slot per thread, added after joining. No shared data while
///        the threads run, so no lock; not a shared counter. The baseline.
[[nodiscard]] std::int64_t countWithResultSlots(const Query& query);

/// @brief Shared atomic counter, increased once per valid number with
///        fetch_add. No lock, but every increment moves the counter's cache line
///        to the core of the thread that increments it.
[[nodiscard]] std::int64_t countAtomicPerHit(const Query& query);

/// @brief Shared counter, locked once per valid number. Correct, but all
///        threads wait for the same lock: shows the cost of contention.
/// @tparam Lock std::mutex, SpinLock or any other BasicLockable type.
template <infrastructure::BasicLockable Lock>
[[nodiscard]] std::int64_t countSharedPerHit(const Query& query) {
    const std::vector<domain::Range> parts =
        domain::splitRange(domain::Range{query.begin, query.end}, query.threadCount);
    infrastructure::SharedCounter<Lock> counter;

    infrastructure::runInParallel(
        parts.size(), [&parts, &counter, modulus = query.modulus](std::size_t index) {
            for (std::int64_t number = parts[index].begin; number < parts[index].end; ++number) {
                if (domain::passesMCheck(number, modulus)) {
                    counter.add(1);  // takes and releases the lock for every hit
                }
            }
        });
    return counter.value();
}

/// @brief Shared counter, locked once per thread: every thread counts its own
///        part locally and adds the result under the lock. The lock is taken p
///        times in total, so it is held as briefly and as rarely as possible.
/// @tparam Lock std::mutex, SpinLock or any other BasicLockable type.
template <infrastructure::BasicLockable Lock>
[[nodiscard]] std::int64_t countSharedPerThread(const Query& query) {
    const std::vector<domain::Range> parts =
        domain::splitRange(domain::Range{query.begin, query.end}, query.threadCount);
    infrastructure::SharedCounter<Lock> counter;

    // LEARN: Capture by reference: parts and counter outlive the threads,
    // LEARN: because runInParallel joins every thread before it returns.
    infrastructure::runInParallel(parts.size(),
                                  [&parts, &counter, modulus = query.modulus](std::size_t index) {
                                      // The counting itself uses only local data; the lock is
                                      // needed only for the one addition at the end.
                                      counter.add(domain::countValid(parts[index], modulus));
                                  });
    return counter.value();
}

}  // namespace iban::application
