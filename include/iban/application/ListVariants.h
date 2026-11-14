/// @file ListVariants.h
/// @brief Ways to write the valid numbers of p threads as numbered lines, kept
///        side by side so their cost can be measured. The program uses
///        listWithLock<std::mutex> (see ParallelList.h); the reasons are in
///        docs/decisions/0012-list-mode-output.md.
// LEARN: Every variant writes the same set of lines; the sequence numbers must
// LEARN: ascend in the order of the lines. They differ in who writes and when.

#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

#include "iban/application/Query.h"
#include "iban/domain/ForEachValid.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"
#include "iban/infrastructure/BasicLockable.h"
#include "iban/infrastructure/ForkJoin.h"
#include "iban/infrastructure/SequencedWriter.h"

namespace iban::application {

/// Default capacity of the buffer between the counting threads and the writer.
inline constexpr std::size_t kDefaultBufferCapacity = 1024;

/// @brief Every thread writes each valid number itself, as soon as it finds it:
///        sequence number and line under one lock.
/// @tparam Lock std::mutex, SpinLock or any other BasicLockable type.
/// @return The number of lines written.
template <infrastructure::BasicLockable Lock>
std::int64_t listWithLock(const Query& query, std::ostream& out) {
    const std::vector<domain::Range> parts =
        domain::splitRange(domain::Range{query.begin, query.end}, query.threadCount);
    infrastructure::SequencedWriter<Lock> writer(out);

    infrastructure::runInParallel(
        parts.size(), [&parts, &writer, modulus = query.modulus](std::size_t index) {
            domain::forEachValid(parts[index], modulus,
                                 [&writer](std::int64_t number) { writer.write(number); });
        });
    return writer.written();
}

/// @brief The counting threads put each valid number in a bounded buffer; one
///        extra thread takes them out, numbers them and writes them
///        (producer-consumer). Only the writer touches the stream and the
///        sequence number, so the lock is held only to add to the buffer.
/// @param capacity Maximum number of numbers waiting in the buffer.
/// @return The number of lines written.
std::int64_t listWithWriterThread(const Query& query, std::ostream& out,
                                  std::size_t capacity = kDefaultBufferCapacity);

}  // namespace iban::application
