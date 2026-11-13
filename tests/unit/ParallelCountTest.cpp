/// @file ParallelCountTest.cpp
/// @brief Unit tests for the parallel count: it must give the same answer as
///        the sequential count for every thread count, including more threads
///        than numbers.
// LEARN: The sequential countValid is the reference: if both agree for many
// LEARN: ranges and thread counts, the split and the joins are correct.

#include <gtest/gtest.h>

#include <cstdint>

#include "iban/application/Limits.h"
#include "iban/application/ParallelCount.h"
#include "iban/application/Query.h"
#include "iban/domain/Count.h"
#include "iban/domain/Range.h"

using iban::application::countParallel;
using iban::application::Query;
using iban::domain::countValid;
using iban::domain::Range;

namespace {

Query makeQuery(std::int64_t begin, std::int64_t end, int modulus, int threads) {
    Query query;
    query.begin = begin;
    query.end = end;
    query.modulus = modulus;
    query.threadCount = threads;
    return query;
}

}  // namespace

TEST(ParallelCount, CountsAssignmentExample) {
    EXPECT_EQ(countParallel(makeQuery(274856170, 274856190, 11, 4)), 2);
}

TEST(ParallelCount, MatchesSequentialForEveryThreadCount) {
    const Range range{1'000'000, 1'100'000};
    for (const int modulus : {1, 7, 11, 12, 256}) {
        const std::int64_t expected = countValid(range, modulus);
        for (int threads = 1; threads <= 16; ++threads) {
            EXPECT_EQ(countParallel(makeQuery(range.begin, range.end, modulus, threads)), expected)
                << "m = " << modulus << ", p = " << threads;
        }
    }
}

TEST(ParallelCount, MoreThreadsThanNumbers) {
    // 3 numbers over 8 threads: five threads get an empty part.
    EXPECT_EQ(countParallel(makeQuery(0, 3, 1, 8)), 3);
}

TEST(ParallelCount, MaximumThreadCount) {
    const Query query = makeQuery(274856170, 274856190, 11, iban::application::kMaxThreads);
    EXPECT_EQ(countParallel(query), 2);
}
