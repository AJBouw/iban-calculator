/// @file ParallelList.cpp
/// @brief List mode with p threads. See ParallelList.h.

#include "iban/application/ParallelList.h"

#include <mutex>

#include "iban/application/ListVariants.h"

namespace iban::application {

std::int64_t listParallel(const Query& query, std::ostream& out) {
    // The variant chosen in docs/decisions/0012-list-mode-output.md: every
    // thread writes its own numbers, sequence number and line under one mutex.
    return listWithLock<std::mutex>(query, out);
}

}  // namespace iban::application
