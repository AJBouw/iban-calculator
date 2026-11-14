/// @file ListOrderDemo.cpp
/// @brief Shows why the sequence number and the line must be one critical
///        section in list mode. Not a unit test: the outcome differs per run.
///
/// Variant B takes the sequence number with an atomic and writes the line under
/// a lock, but in two separate steps. Every single access is synchronised, so
/// ThreadSanitizer reports nothing, yet a thread can take number 3, another
/// thread number 4, and the second one can write first. This is a race
/// condition without a data race: a logic error in the order of operations.
///   ./build/debug/iban_list_order_demo
///   ./build/tsan/iban_list_order_demo      (no ThreadSanitizer warning)
// LEARN: A data race is two unsynchronised accesses to the same memory; a race
// LEARN: condition is a wrong result that depends on timing. Locks and atomics
// LEARN: remove data races, but only the right critical sections remove race
// LEARN: conditions.

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

#include "iban/domain/ForEachValid.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"
#include "iban/infrastructure/ForkJoin.h"
#include "iban/infrastructure/SequencedWriter.h"

namespace {

using iban::domain::forEachValid;
using iban::domain::Range;
using iban::domain::splitRange;
using iban::infrastructure::runInParallel;

constexpr Range kRange{0, 200'000};
constexpr int kModulus = 1;  // every number is valid: one line per number
constexpr int kThreads = 8;

// Number of places where a line has a lower sequence number than the line
// before it.
std::int64_t countDescents(const std::vector<std::int64_t>& sequences) {
    std::int64_t descents = 0;
    for (std::size_t index = 1; index < sequences.size(); ++index) {
        if (sequences[index] < sequences[index - 1]) {
            ++descents;
        }
    }
    return descents;
}

// Variant B: atomic sequence number, then the line under a lock, in two steps.
std::vector<std::int64_t> writeInTwoSteps() {
    const auto parts = splitRange(kRange, kThreads);
    std::atomic<std::int64_t> nextSequence{0};
    std::mutex outputLock;
    std::vector<std::int64_t> writtenSequences;  // stands for the output stream
    writtenSequences.reserve(static_cast<std::size_t>(kRange.size()));

    runInParallel(parts.size(), [&](std::size_t index) {
        forEachValid(parts[index], kModulus, [&](std::int64_t /*number*/) {
            const std::int64_t sequence = nextSequence.fetch_add(1) + 1;  // step 1
            // Another thread can take the next number and write it right here.
            std::lock_guard guard(outputLock);  // step 2
            writtenSequences.push_back(sequence);
        });
    });
    return writtenSequences;
}

// Variant A, as in the program: the sequence number and the line are one
// critical section (SequencedWriter).
std::vector<std::int64_t> writeInOneStep() {
    const auto parts = splitRange(kRange, kThreads);
    std::ostringstream out;
    iban::infrastructure::SequencedWriter writer(out);
    runInParallel(parts.size(), [&](std::size_t index) {
        forEachValid(parts[index], kModulus,
                     [&writer](std::int64_t number) { writer.write(number); });
    });

    std::vector<std::int64_t> sequences;
    std::istringstream lines(out.str());
    std::int64_t sequence = 0;
    std::int64_t number = 0;
    while (lines >> sequence >> number) {
        sequences.push_back(sequence);
    }
    return sequences;
}

}  // namespace

int main() {
    std::cout << "Writing " << kRange.size() << " numbered lines with " << kThreads
              << " threads.\n\n";

    std::cout << "B. Atomic sequence number, then the line under a lock (two steps):\n";
    for (int run = 1; run <= 5; ++run) {
        std::cout << "  run " << run << ": " << countDescents(writeInTwoSteps())
                  << " lines with a lower sequence number than the line before\n";
    }

    std::cout << "\nA. Sequence number and line under one lock (the program):\n";
    for (int run = 1; run <= 5; ++run) {
        std::cout << "  run " << run << ": " << countDescents(writeInOneStep())
                  << " lines with a lower sequence number than the line before\n";
    }
    return 0;
}
