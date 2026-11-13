/// @file CountVariants.cpp
/// @brief The count variants that are not templates. See CountVariants.h.

#include "iban/application/CountVariants.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

#include "iban/domain/Count.h"
#include "iban/domain/MCheck.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"
#include "iban/infrastructure/ForkJoin.h"

namespace iban::application {

std::int64_t countWithResultSlots(const Query& query) {
    const std::vector<domain::Range> parts =
        domain::splitRange(domain::Range{query.begin, query.end}, query.threadCount);

    // Thread i writes only counts[i]. Different elements are in different memory
    // locations, so there is no data race.
    std::vector<std::int64_t> counts(parts.size(), 0);
    infrastructure::runInParallel(parts.size(),
                                  [&parts, &counts, modulus = query.modulus](std::size_t index) {
                                      counts[index] = domain::countValid(parts[index], modulus);
                                  });
    // LEARN: std::accumulate adds all elements; the start value sets the type.
    return std::accumulate(counts.begin(), counts.end(), std::int64_t{0});
}

std::int64_t countAtomicPerHit(const Query& query) {
    const std::vector<domain::Range> parts =
        domain::splitRange(domain::Range{query.begin, query.end}, query.threadCount);

    std::atomic<std::int64_t> counter{0};
    infrastructure::runInParallel(
        parts.size(), [&parts, &counter, modulus = query.modulus](std::size_t index) {
            for (std::int64_t number = parts[index].begin; number < parts[index].end; ++number) {
                if (domain::passesMCheck(number, modulus)) {
                    // relaxed is enough: only the sum matters, and it is read
                    // after join, which makes every increment visible.
                    // LEARN: fetch_add is one indivisible read-modify-write, so
                    // LEARN: no increment of another thread can be lost.
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    return counter.load(std::memory_order_relaxed);
}

}  // namespace iban::application
