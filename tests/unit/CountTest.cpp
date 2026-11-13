/// @file CountTest.cpp
/// @brief Unit tests for the sequential count: the assignment examples, empty
///        ranges, the trivial modulus and invalid arguments.

#include <gtest/gtest.h>

#include <stdexcept>

#include "iban/domain/Count.h"
#include "iban/domain/Range.h"

using iban::domain::countValid;
using iban::domain::Range;

TEST(CountValid, CountsAssignmentExample) {
    // Assignment: "0 274856170 274856190 11 4 0 0" gives 2.
    EXPECT_EQ(countValid(Range{274856170, 274856190}, 11), 2);
}

TEST(CountValid, CountsTwelveCheckExample) {
    // Assignment: the list example with m = 12 shows six numbers.
    EXPECT_EQ(countValid(Range{374856170, 374856250}, 12), 6);
}

TEST(CountValid, EmptyRangeCountsNothing) {
    EXPECT_EQ(countValid(Range{100, 100}, 11), 0);
}

TEST(CountValid, ModulusOneCountsEveryNumber) {
    // Every sum is divisible by 1.
    EXPECT_EQ(countValid(Range{0, 1000}, 1), 1000);
}

TEST(CountValid, RejectsModulusBelowOne) {
    // Rejected even for an empty range, before the loop starts.
    EXPECT_THROW((void)countValid(Range{0, 0}, 0), std::invalid_argument);
}
