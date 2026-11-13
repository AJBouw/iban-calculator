/// @file RaceDemo.cpp
/// @brief Shows the data race of a shared counter without a lock. Not a unit
///        test: a race is not reproducible, so a test that expects a wrong
///        result would sometimes pass and sometimes fail.
///
/// Run it in a Debug build to see lost updates, and in the tsan build to see
/// ThreadSanitizer report the race:
///   ./build/debug/iban_race_demo
///   ./build/tsan/iban_race_demo
/// In a Release build the optimiser may combine the increments of one thread
/// into one write, which hides most lost updates. The data race is still there:
/// it is undefined behaviour, and ThreadSanitizer still reports it.
// LEARN: A data race: two threads access the same memory location at the same
// LEARN: time, at least one writes, and nothing orders the accesses.

#include <cstdint>
#include <iostream>
#include <mutex>

#include "iban/application/CountVariants.h"
#include "iban/application/Query.h"
#include "iban/domain/Count.h"
#include "iban/domain/Range.h"
#include "iban/infrastructure/SpinLock.h"

namespace {

namespace app = iban::application;

/// Satisfies BasicLockable but does nothing: SharedCounter<NoLock> is a shared
/// counter without protection. Only for this demonstration.
struct NoLock {
    void lock() noexcept {}
    void unlock() noexcept {}
};

}  // namespace

int main() {
    // m = 1: every number is valid, so every number is an increment of the
    // shared counter and the threads collide as often as possible.
    app::Query query;
    query.begin = 0;
    query.end = 2'000'000;
    query.modulus = 1;
    query.threadCount = 8;

    const std::int64_t expected =
        iban::domain::countValid(iban::domain::Range{query.begin, query.end}, query.modulus);
    std::cout << "Counting [" << query.begin << ", " << query.end << ") with m = " << query.modulus
              << " and p = " << query.threadCount << "; expected " << expected << ".\n\n";

    std::cout << "Without a lock (data race):\n";
    for (int run = 1; run <= 5; ++run) {
        const std::int64_t counted = app::countSharedPerHit<NoLock>(query);
        std::cout << "  run " << run << ": " << counted << "  (" << expected - counted
                  << " increments lost)\n";
    }

    std::cout << "\nWith std::mutex:    " << app::countSharedPerHit<std::mutex>(query) << '\n';
    std::cout << "With SpinLock:      "
              << app::countSharedPerHit<iban::infrastructure::SpinLock>(query) << '\n';
    return 0;
}
