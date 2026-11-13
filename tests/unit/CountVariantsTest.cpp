/// @file CountVariantsTest.cpp
/// @brief Every way of combining the thread counts must give the same answer as
///        the sequential count, for every thread count and modulus.
// LEARN: The variants differ only in synchronisation; if one of them were wrong
// LEARN: (a lost update, a part counted twice), its count would differ here.

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "iban/application/CountVariants.h"
#include "iban/application/Limits.h"
#include "iban/application/Query.h"
#include "iban/domain/Count.h"
#include "iban/domain/Range.h"
#include "iban/infrastructure/SpinLock.h"

namespace app = iban::application;
using iban::domain::countValid;
using iban::domain::Range;
using iban::infrastructure::SpinLock;

namespace {

using CountFunction = std::function<std::int64_t(const app::Query&)>;

// Name and function of every variant, so a failure names the variant.
const std::vector<std::pair<std::string, CountFunction>>& variants() {
    static const std::vector<std::pair<std::string, CountFunction>> list = {
        {"result slots", app::countWithResultSlots},
        {"atomic per hit", app::countAtomicPerHit},
        {"mutex per hit", app::countSharedPerHit<std::mutex>},
        {"spin lock per hit", app::countSharedPerHit<SpinLock>},
        {"mutex per thread", app::countSharedPerThread<std::mutex>},
        {"spin lock per thread", app::countSharedPerThread<SpinLock>},
    };
    return list;
}

app::Query makeQuery(Range range, int modulus, int threads) {
    app::Query query;
    query.begin = range.begin;
    query.end = range.end;
    query.modulus = modulus;
    query.threadCount = threads;
    return query;
}

}  // namespace

TEST(CountVariants, CountAssignmentExample) {
    for (const auto& [name, count] : variants()) {
        EXPECT_EQ(count(makeQuery(Range{274856170, 274856190}, 11, 4)), 2) << name;
    }
}

TEST(CountVariants, MatchSequentialForEveryThreadCount) {
    const Range range{1'000'000, 1'050'000};
    // m = 1 makes every number valid: the most additions to the shared counter.
    for (const int modulus : {1, 11, 256}) {
        const std::int64_t expected = countValid(range, modulus);
        for (const auto& [name, count] : variants()) {
            for (const int threads : {1, 2, 3, 8, 16}) {
                EXPECT_EQ(count(makeQuery(range, modulus, threads)), expected)
                    << name << ", m = " << modulus << ", p = " << threads;
            }
        }
    }
}

TEST(CountVariants, MoreThreadsThanNumbers) {
    // 3 numbers over the maximum number of threads: most parts are empty.
    for (const auto& [name, count] : variants()) {
        EXPECT_EQ(count(makeQuery(Range{0, 3}, 1, app::kMaxThreads)), 3) << name;
    }
}
