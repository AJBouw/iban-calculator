/// @file ParallelSearch.cpp
/// @brief The program's search mode: the stop flag checked before every number.

#include "iban/application/ParallelSearch.h"

#include "iban/application/SearchVariants.h"

namespace iban::application {

// An interval of 1 stops the other threads after at most one more number; the
// measurement in docs/measurements.md compares it with larger intervals.
std::int64_t searchParallel(const Query& query) {
    return searchWithStopFlag<1>(query);
}

}  // namespace iban::application
