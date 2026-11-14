/// @file SearchVariantsTest.cpp
/// @brief Every search variant must give s when it lies in [b, e) and passes
///        the m-check, and -1 otherwise, for every thread count. The variants
///        that look at the stop flag must also stop the other threads.
// LEARN: The answer does not depend on when the threads stop, so every variant,
// LEARN: also the one without a stop flag, must give exactly the same result.

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "iban/application/Limits.h"
#include "iban/application/ParallelSearch.h"
#include "iban/application/Query.h"
#include "iban/application/SearchVariants.h"
#include "iban/domain/MCheck.h"
#include "iban/domain/Range.h"

namespace app = iban::application;
using iban::domain::passesMCheck;
using iban::domain::Range;

namespace {

using SearchFunction = std::function<std::int64_t(const app::Query&)>;
using NamedSearch = std::pair<std::string, SearchFunction>;

const std::vector<NamedSearch>& stoppingVariants() {
    static const std::vector<NamedSearch> list = {
        {"program", app::searchParallel},
        {"every number", app::searchWithStopFlag<1>},
        {"every 1024 numbers", app::searchWithStopFlag<1024>},
    };
    return list;
}

const std::vector<NamedSearch>& allVariants() {
    static const std::vector<NamedSearch> list = [] {
        std::vector<NamedSearch> variants = stoppingVariants();
        variants.emplace_back("never stop", app::searchWithStopFlag<app::kNeverStop>);
        return variants;
    }();
    return list;
}

app::Query makeQuery(Range range, int modulus, int threads, std::int64_t target) {
    app::Query query;
    query.begin = range.begin;
    query.end = range.end;
    query.modulus = modulus;
    query.threadCount = threads;
    query.mode = app::Mode::Search;
    query.target = target;
    return query;
}

// The range of the assignment example: 274856174 and 274856182 pass the 11-check.
constexpr Range kExample{274856170, 274856190};

}  // namespace

TEST(SearchVariants, FindTheValidNumbersOfTheAssignmentExample) {
    for (const auto& [name, search] : allVariants()) {
        EXPECT_EQ(search(makeQuery(kExample, 11, 4, 274856182)), 274856182) << name;
        EXPECT_EQ(search(makeQuery(kExample, 11, 4, 274856174)), 274856174) << name;
    }
}

TEST(SearchVariants, GiveMinusOneForANumberThatFailsTheMCheck) {
    for (const auto& [name, search] : allVariants()) {
        EXPECT_EQ(search(makeQuery(kExample, 11, 4, 274856183)), app::kNotFound) << name;
    }
}

TEST(SearchVariants, GiveMinusOneForANumberOutsideTheRange) {
    for (const auto& [name, search] : allVariants()) {
        for (const std::int64_t target :
             {std::int64_t{274856169}, kExample.end, std::int64_t{-5}}) {
            EXPECT_EQ(search(makeQuery(kExample, 11, 4, target)), app::kNotFound)
                << name << ", s = " << target;
        }
    }
}

// b is inclusive and e exclusive: in [274856174, 274856183) both ends are valid.
TEST(SearchVariants, FindTheNumbersAtBothEndsOfTheRange) {
    const Range range{274856174, 274856183};
    for (const auto& [name, search] : allVariants()) {
        EXPECT_EQ(search(makeQuery(range, 11, 4, range.begin)), range.begin) << name;
        EXPECT_EQ(search(makeQuery(range, 11, 4, range.end - 1)), range.end - 1) << name;
    }
}

// Every s in and around a small range, for several thread counts, compared with
// the m-check itself.
TEST(SearchVariants, AgreeWithTheMCheckForEveryTargetAndThreadCount) {
    const Range range{1'000, 1'200};
    constexpr int kModulus = 7;
    for (const auto& [name, search] : allVariants()) {
        for (const int threads : {1, 2, 3, 8, 16}) {
            for (std::int64_t target = range.begin - 5; target < range.end + 5; ++target) {
                const bool inRange = target >= range.begin && target < range.end;
                const std::int64_t expected =
                    inRange && passesMCheck(target, kModulus) ? target : app::kNotFound;
                EXPECT_EQ(search(makeQuery(range, kModulus, threads, target)), expected)
                    << name << ", p = " << threads << ", s = " << target;
            }
        }
    }
}

TEST(SearchVariants, MoreThreadsThanNumbers) {
    for (const auto& [name, search] : allVariants()) {
        EXPECT_EQ(search(makeQuery(Range{0, 3}, 1, app::kMaxThreads, 2)), 2) << name;
    }
}

TEST(SearchVariants, RequireTheTarget) {
    app::Query query = makeQuery(kExample, 11, 4, 274856182);
    query.target.reset();
    for (const auto& [name, search] : allVariants()) {
        EXPECT_THROW(static_cast<void>(search(query)), std::invalid_argument) << name;
    }
}

// s = 0 is the first number of the largest range, so the first thread finds it
// at once. Without stopping, the other seven threads would each check more than
// 250 million numbers, which takes seconds; with the stop flag the test ends
// almost immediately.
// LEARN: A test that only passes quickly when the threads stop: if stopping
// LEARN: breaks, this test becomes very slow instead of failing.
TEST(SearchVariants, StopTheOtherThreadsWhenTheNumberIsFound) {
    const Range largest{app::kMinBound, app::kMaxBound};
    for (const auto& [name, search] : stoppingVariants()) {
        EXPECT_EQ(search(makeQuery(largest, 11, 8, 0)), 0) << name;
    }
}
