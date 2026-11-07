/// @file MCheckTest.cpp
/// @brief Unit tests for the m-check: the examples from the assignment, values
///        that are easy to verify by hand, and edge cases.

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

#include "iban/domain/MCheck.h"

// LEARN: `using` makes a name available without its full namespace prefix,
// LEARN: so you can write weightedDigitSum instead of iban::domain::weightedDigitSum.
using iban::domain::passesMCheck;
using iban::domain::weightedDigitSum;

namespace {
// Upper bound from the assignment (2^31 - 1).
constexpr std::int64_t kMaxBound = 2147483647;
}  // namespace

// ---------------------------------------------------------------------------
// weightedDigitSum
// ---------------------------------------------------------------------------

// Example from the assignment: 23.12.43.138 has weighted sum 110.
TEST(WeightedDigitSum, AssignmentExample) {
    // 2*9 + 3*8 + 1*7 + 2*6 + 4*5 + 3*4 + 1*3 + 3*2 + 8*1 = 110
    EXPECT_EQ(weightedDigitSum(231243138), 110);
}

// Small values that can be verified by hand.
TEST(WeightedDigitSum, LastDigitHasWeightOne) {
    EXPECT_EQ(weightedDigitSum(7), 7);
    EXPECT_EQ(weightedDigitSum(10), 2);    // 1*2 + 0*1
    EXPECT_EQ(weightedDigitSum(123), 10);  // 1*3 + 2*2 + 3*1
}

// The assignment states numbers need not have 9 digits.
TEST(WeightedDigitSum, WorksForAnyNumberOfDigits) {
    EXPECT_EQ(weightedDigitSum(1234567), 84);  // 7 digits
    // 2147483647 (10 digits, the largest allowed input)
    EXPECT_EQ(weightedDigitSum(kMaxBound),
              2 * 10 + 1 * 9 + 4 * 8 + 7 * 7 + 4 * 6 + 8 * 5 + 3 * 4 + 6 * 3 + 4 * 2 + 7 * 1);
}

TEST(WeightedDigitSum, ZeroIsZero) {
    EXPECT_EQ(weightedDigitSum(0), 0);
}

TEST(WeightedDigitSum, NegativeNumberIsRejected) {
    // LEARN: EXPECT_THROW checks that the code throws an exception of this type.
    EXPECT_THROW(weightedDigitSum(-1), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// passesMCheck
// ---------------------------------------------------------------------------

// Valid and invalid numbers from the assignment examples (m = 11).
TEST(PassesMCheck, AssignmentExamplesModulus11) {
    EXPECT_TRUE(passesMCheck(231243138, 11));
    EXPECT_TRUE(passesMCheck(274856174, 11));
    EXPECT_TRUE(passesMCheck(274856182, 11));
    EXPECT_FALSE(passesMCheck(274856183, 11));
}

// Valid and invalid numbers from the assignment examples (m = 12).
TEST(PassesMCheck, AssignmentExamplesModulus12) {
    for (const std::int64_t number :
         {374856173, 374856181, 374856202, 374856210, 374856238, 374856246}) {
        // LEARN: << after an EXPECT adds extra text to the failure message.
        EXPECT_TRUE(passesMCheck(number, 12)) << number;
    }
    EXPECT_FALSE(passesMCheck(374856174, 12));
}

// Every number is a multiple of 1.
TEST(PassesMCheck, ModulusOneAcceptsEverything) {
    for (std::int64_t x = 0; x < 1000; ++x) {
        EXPECT_TRUE(passesMCheck(x, 1)) << x;
    }
}

// Largest modulus from the assignment: the sum must reach a multiple of 256.
TEST(PassesMCheck, LargestModulus) {
    EXPECT_FALSE(passesMCheck(1, 256));       // sum 1
    EXPECT_FALSE(passesMCheck(kMaxBound, 256));  // sum 244, smaller than 256
    // 10399999: 1*8 + 0*7 + 3*6 + 9*(5+4+3+2+1) = 8 + 0 + 18 + 135 = 161 ... see below
    EXPECT_EQ(passesMCheck(99999999, 256), weightedDigitSum(99999999) % 256 == 0);
}

// x % 0 is undefined behaviour, so a non-positive modulus must be rejected.
TEST(PassesMCheck, NonPositiveModulusIsRejected) {
    EXPECT_THROW(passesMCheck(42, 0), std::invalid_argument);
    EXPECT_THROW(passesMCheck(42, -11), std::invalid_argument);
}

// Documents the current choice; see the open question in MCheck.h.
TEST(PassesMCheck, ZeroCountsAsValid) {
    EXPECT_TRUE(passesMCheck(0, 11));
    EXPECT_TRUE(passesMCheck(0, 256));
}