/// @file MCheckTest.cpp
/// @brief Unit tests for the m-check: known sums, the boundaries of the
///        assignment, error detection and invalid arguments.
// LEARN: Each TEST(Suite, Name) is a separate test; CTest reports each one.
// LEARN: EXPECT_* continues after a failure, ASSERT_* stops the current test.

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "iban/domain/MCheck.h"

using iban::domain::passesMCheck;
using iban::domain::weightedDigitSum;
// LEARN: using-declarations: write passesMCheck instead of iban::domain::passesMCheck.
// LEARN: Only in .cpp files; in a header they would leak into every includer.

namespace {

// The valid number from the assignment example, used as a base for mutations.
constexpr std::int64_t kAssignmentNumber = 274856182;
constexpr int kElevenCheck = 11;

// Text helpers for the mutation tests: changing a digit is easiest as text.
// LEARN: std::stoll converts text back to a 64-bit integer (long long).
std::int64_t toNumber(const std::string& digits) {
    return std::stoll(digits);
}

}  // namespace

// ---------------------------------------------------------------------------
// Weighted digit sum: known values, checked by hand
// ---------------------------------------------------------------------------

// One row per case, so a failure names the exact number.
// LEARN: A struct with two fields; the test below runs once per row.
struct SumCase {
    std::int64_t number;
    int expectedSum;
};

// Readable test names in CTest and in failure messages, instead of raw bytes.
// LEARN: GoogleTest looks for a function named PrintTo to print custom types.
void PrintTo(const SumCase& testCase, std::ostream* out) {
    *out << testCase.number << " has sum " << testCase.expectedSum;
}

// LEARN: TEST_P is a parameterised test: one test body, many inputs.
class WeightedDigitSumTest : public ::testing::TestWithParam<SumCase> {};

TEST_P(WeightedDigitSumTest, MatchesHandCalculation) {
    const SumCase& testCase = GetParam();
    EXPECT_EQ(weightedDigitSum(testCase.number), testCase.expectedSum)
        << "number " << testCase.number;
}

// LEARN: INSTANTIATE_TEST_SUITE_P supplies the rows; each becomes its own test.
INSTANTIATE_TEST_SUITE_P(KnownValues, WeightedDigitSumTest,
                         ::testing::Values(SumCase{274856182, 220},    // assignment example
                                           SumCase{123456789, 165},    // 15 * 11
                                           SumCase{0, 0},              // no digits
                                           SumCase{7, 7},              // weight 1
                                           SumCase{10, 2},             // 1 at weight 2
                                           SumCase{19998999, 256},     // sum exceeds one byte
                                           SumCase{2147483647, 219},   // 2^31-1, upper bound
                                           SumCase{9999999999, 495}),  // largest 10-digit sum
                         // LEARN: Lambda that names each case after its number.
                         [](const ::testing::TestParamInfo<SumCase>& caseInfo) {
                             return std::to_string(caseInfo.param.number);
                         });

// ---------------------------------------------------------------------------
// The check itself
// ---------------------------------------------------------------------------

TEST(PassesMCheck, AcceptsAssignmentExample) {
    EXPECT_TRUE(passesMCheck(kAssignmentNumber, kElevenCheck));
}

TEST(PassesMCheck, RejectsNeighbourOfValidNumber) {
    // 274856183 has sum 221, which 11 does not divide.
    EXPECT_FALSE(passesMCheck(kAssignmentNumber + 1, kElevenCheck));
}

TEST(PassesMCheck, ModulusOneAcceptsEveryNumber) {
    // Every sum is divisible by 1: the weakest possible check.
    for (std::int64_t number = 0; number < 1000; ++number) {
        EXPECT_TRUE(passesMCheck(number, 1)) << "number " << number;
    }
}

TEST(PassesMCheck, ZeroPassesEveryModulus) {
    // Sum 0 is divisible by every m (documented behaviour of the function).
    EXPECT_TRUE(passesMCheck(0, 1));
    EXPECT_TRUE(passesMCheck(0, kElevenCheck));
    EXPECT_TRUE(passesMCheck(0, 256));
}

// ---------------------------------------------------------------------------
// Error detection: why m = 11 is the classic choice
// ---------------------------------------------------------------------------

// Changing one digit by delta changes the sum by weight * delta. Both are
// below 11 and 11 is prime, so the change is never a multiple of 11.
TEST(PassesMCheck, ElevenCheckDetectsEverySingleDigitError) {
    const std::string original = std::to_string(kAssignmentNumber);
    for (std::size_t position = 0; position < original.size(); ++position) {
        for (char digit = '0'; digit <= '9'; ++digit) {
            if (digit == original[position] || (position == 0 && digit == '0')) {
                continue;  // not an error, or a leading zero (a shorter number)
            }
            std::string mutated = original;
            mutated[position] = digit;
            EXPECT_FALSE(passesMCheck(toNumber(mutated), kElevenCheck)) << "mutated " << mutated;
        }
    }
}

// Swapping neighbours a and b changes the sum by exactly (a - b), which is
// between -9 and 9 and therefore never a non-zero multiple of 11.
TEST(PassesMCheck, ElevenCheckDetectsEveryAdjacentSwap) {
    const std::string original = std::to_string(kAssignmentNumber);
    for (std::size_t position = 0; position + 1 < original.size(); ++position) {
        if (original[position] == original[position + 1]) {
            continue;  // swapping equal digits changes nothing
        }
        std::string swapped = original;
        // LEARN: std::swap exchanges two values in place.
        std::swap(swapped[position], swapped[position + 1]);
        EXPECT_FALSE(passesMCheck(toNumber(swapped), kElevenCheck)) << "swapped " << swapped;
    }
}

// ---------------------------------------------------------------------------
// Invalid arguments: caller bugs are reported, not turned into false
// ---------------------------------------------------------------------------

TEST(PassesMCheck, RejectsModulusBelowOne) {
    // x % 0 is undefined behaviour, so 0 must never reach the division.
    // LEARN: EXPECT_THROW passes only if the call throws exactly this type.
    EXPECT_THROW((void)passesMCheck(kAssignmentNumber, 0), std::invalid_argument);
    EXPECT_THROW((void)passesMCheck(kAssignmentNumber, -11), std::invalid_argument);
}

TEST(PassesMCheck, RejectsNegativeNumber) {
    // LEARN: (void) discards the result on purpose, which silences [[nodiscard]].
    EXPECT_THROW((void)weightedDigitSum(-1), std::invalid_argument);
    EXPECT_THROW((void)passesMCheck(-1, kElevenCheck), std::invalid_argument);
}
