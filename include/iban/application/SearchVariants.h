/// @file SearchVariants.h
/// @brief Search mode with p threads and a shared stop flag, with the interval
///        at which a thread looks at the flag as a parameter, so the cost and
///        the benefit of stopping can be measured. The program uses an
///        interval of 1 (see ParallelSearch.h); the reasons are in
///        docs/decisions/0014-stopping-the-search.md.
// LEARN: The flag is a local variable of one search, not a global: every search
// LEARN: has its own flag, which the thread pool will need later.

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "iban/application/Query.h"
#include "iban/domain/FindValid.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"
#include "iban/infrastructure/ForkJoin.h"

namespace iban::application {

/// Printed by search mode when the account number is not found.
inline constexpr std::int64_t kNotFound = -1;

/// Check interval of the baseline: the threads never look at the stop flag and
/// always search their whole part.
inline constexpr std::int64_t kNeverStop = 0;

/// @brief Searches [query.begin, query.end) for the valid account number s
///        with query.threadCount threads; the thread that finds it tells the
///        others to stop.
/// @tparam CheckInterval A thread looks at the stop flag before every
///         CheckInterval-th number; kNeverStop means never.
/// @param query A validated query with a target s.
/// @return s when it lies in the range and passes the m-check; otherwise,
///         kNotFound.
/// @throws std::invalid_argument when the query has no target.
/// @throws std::system_error when a thread cannot be started; threads that
///         were already started are joined first.
template <std::int64_t CheckInterval>
std::int64_t searchWithStopFlag(const Query& query) {
    static_assert(CheckInterval >= 0, "the check interval cannot be negative");
    if (!query.target) {
        throw std::invalid_argument("search mode requires s");
    }
    const std::int64_t target = *query.target;
    const std::vector<domain::Range> parts =
        domain::splitRange(domain::Range{query.begin, query.end}, query.threadCount);

    // Relaxed is enough for both atomics: they only signal "stop" and hold the
    // answer, and runInParallel joins every thread before the answer is read.
    // The join makes all writes of a thread visible to the caller.
    // LEARN: memory_order_relaxed only guarantees that the access itself is atomic;
    // LEARN: it orders nothing else. Nothing else needs ordering here: the join does that.
    std::atomic<bool> stop{false};
    std::atomic<std::int64_t> found{kNotFound};

    infrastructure::runInParallel(parts.size(), [&](std::size_t index) {
        std::int64_t untilCheck = CheckInterval;
        const auto stopRequested = [&]() {
            if constexpr (CheckInterval == kNeverStop) {
                return false;
            } else {
                // Counting down is cheaper than a modulo for every number.
                if (--untilCheck > 0) {
                    return false;
                }
                untilCheck = CheckInterval;
                return stop.load(std::memory_order_relaxed);
            }
        };
        const std::optional<std::int64_t> match = domain::findValid(
            parts[index], query.modulus, [target](std::int64_t number) { return number == target; },
            stopRequested);
        if (match) {
            // Only the first answer is kept; with this meaning of s at most one
            // thread can find a number, but the search does not rely on that.
            // LEARN: compare_exchange_strong writes *match only if found still holds
            // LEARN: kNotFound; otherwise it leaves found alone and updates expected.
            std::int64_t expected = kNotFound;
            static_cast<void>(
                found.compare_exchange_strong(expected, *match, std::memory_order_relaxed));
            stop.store(true, std::memory_order_relaxed);
        }
    });
    return found.load(std::memory_order_relaxed);
}

}  // namespace iban::application
