/// @file ForEachValidTest.cpp
/// @brief Unit tests for forEachValid: it visits exactly the numbers that
///        countValid counts, in ascending order.

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "iban/domain/Count.h"
#include "iban/domain/ForEachValid.h"
#include "iban/domain/Range.h"

using iban::domain::countValid;
using iban::domain::forEachValid;
using iban::domain::Range;

namespace {

std::vector<std::int64_t> collect(Range range, int modulus) {
    std::vector<std::int64_t> numbers;
    forEachValid(range, modulus, [&numbers](std::int64_t number) { numbers.push_back(number); });
    return numbers;
}

}  // namespace

TEST(ForEachValid, VisitsTheAssignmentExampleInAscendingOrder) {
    EXPECT_EQ(collect(Range{274856170, 274856190}, 11),
              (std::vector<std::int64_t>{274856174, 274856182}));
}

TEST(ForEachValid, VisitsTheTwelveCheckExample) {
    EXPECT_EQ(collect(Range{374856170, 374856250}, 12),
              (std::vector<std::int64_t>{374856173, 374856181, 374856202, 374856210, 374856238,
                                         374856246}));
}

TEST(ForEachValid, VisitsAsManyNumbersAsCountValidCounts) {
    const Range range{1'000'000, 1'010'000};
    for (const int modulus : {1, 7, 11, 256}) {
        EXPECT_EQ(static_cast<std::int64_t>(collect(range, modulus).size()),
                  countValid(range, modulus))
            << "m = " << modulus;
    }
}

TEST(ForEachValid, EmptyRangeVisitsNothing) {
    EXPECT_TRUE(collect(Range{5, 5}, 1).empty());
}

TEST(ForEachValid, RejectsModulusBelowOne) {
    EXPECT_THROW(collect(Range{0, 10}, 0), std::invalid_argument);
}
