/// @file ParallelCount.cpp
/// @brief Implementation of the parallel count. See ParallelCount.h.

#include "iban/application/ParallelCount.h"

#include <mutex>

#include "iban/application/CountVariants.h"

namespace iban::application {

std::int64_t countParallel(const Query& query) {
    // The variant chosen in docs/decisions/0010-shared-counter-and-lock.md: a
    // shared counter, locked once per thread, with std::mutex.
    return countSharedPerThread<std::mutex>(query);
}

}  // namespace iban::application
