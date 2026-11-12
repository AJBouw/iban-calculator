/// @file InputParserTest.cpp
/// @brief Unit tests for the input parser: the examples from the assignment,
///        the three accepted forms, the boundary values and one test per rule.
// LEARN: These tests call parseInput directly with a string, so no console
// LEARN: input is needed: the parser is tested apart from main().

#include <gtest/gtest.h>

#include <string>

#include "iban/application/Limits.h"
#include "iban/application/Mode.h"
#include "iban/presentation/InputParser.h"

using iban::application::kMaxBound;
using iban::application::kMaxModulus;
using iban::application::kMaxThreads;
using iban::application::Mode;
using iban::presentation::parseInput;

namespace {

// Expects `line` to be rejected with a message that contains `fragment`.
// LEARN: A helper keeps each rejection test to one readable line.
void expectError(const std::string& line, const std::string& fragment) {
    const auto result = parseInput(line);
    EXPECT_FALSE(result.query.has_value()) << "accepted: '" << line << "'";
    // LEARN: find returns npos when the fragment does not occur in the message.
    EXPECT_NE(result.error.find(fragment), std::string::npos)
        << "line '" << line << "' gave error '" << result.error << "'";
}

}  // namespace

// ---------------------------------------------------------------------------
// The examples from the assignment
// ---------------------------------------------------------------------------

TEST(InputParser, AcceptsAssignmentCountExample) {
    const auto result = parseInput("0 274856170 274856190 11 4 0 0");
    // LEARN: ASSERT stops here when there is no query, so *result.query below
    // LEARN: is never reached with an empty optional.
    ASSERT_TRUE(result.query) << result.error;
    const auto& query = *result.query;
    EXPECT_EQ(query.begin, 274856170);
    EXPECT_EQ(query.end, 274856190);
    EXPECT_EQ(query.modulus, 11);
    EXPECT_EQ(query.threadCount, 4);
    EXPECT_EQ(query.mode, Mode::Count);
    EXPECT_EQ(query.target, 0);
}

TEST(InputParser, AcceptsAssignmentListExamples) {
    const auto eleven = parseInput("0 274856170 274856190 11 4 1 0");
    ASSERT_TRUE(eleven.query) << eleven.error;
    EXPECT_EQ(eleven.query->mode, Mode::List);

    const auto twelve = parseInput("0 374856170 374856250 12 4 1 0");
    ASSERT_TRUE(twelve.query) << twelve.error;
    EXPECT_EQ(twelve.query->modulus, 12);
}

// ---------------------------------------------------------------------------
// The three accepted forms
// ---------------------------------------------------------------------------

// The written specification: b e m p u s.
TEST(InputParser, AcceptsSixNumbers) {
    const auto result = parseInput("10 20 7 2 2 15");
    ASSERT_TRUE(result.query) << result.error;
    EXPECT_EQ(result.query->begin, 10);
    EXPECT_EQ(result.query->mode, Mode::Search);
    EXPECT_EQ(result.query->target, 15);
}

// s is optional when it is not needed (count and list mode).
TEST(InputParser, AcceptsFiveNumbersWithoutTarget) {
    const auto result = parseInput("10 20 7 2 0");
    ASSERT_TRUE(result.query) << result.error;
    EXPECT_FALSE(result.query->target.has_value());
}

// Tabs, repeated spaces and a Windows line ending (\r) are accepted.
TEST(InputParser, ToleratesExtraWhitespace) {
    const auto result = parseInput("  0\t274856170   274856190 11 4 0 0\r\n");
    ASSERT_TRUE(result.query) << result.error;
    EXPECT_EQ(result.query->end, 274856190);
}

// A leading zero inside a number does not change its value.
TEST(InputParser, AcceptsLeadingZerosInNumbers) {
    const auto result = parseInput("0274856170 0274856190 11 4 0");
    ASSERT_TRUE(result.query) << result.error;
    EXPECT_EQ(result.query->begin, 274856170);
}

// ---------------------------------------------------------------------------
// Boundary values from the assignment
// ---------------------------------------------------------------------------

TEST(InputParser, AcceptsSmallestAndLargestValues) {
    // Smallest values: b = 0, e = 1, m = 1, p = 1.
    EXPECT_TRUE(parseInput("0 1 1 1 0").query);
    // Largest values: e = 2^31 - 1, m = 256, p = 256, built from Limits.h.
    const std::string largest = std::to_string(kMaxBound - 1) + " " + std::to_string(kMaxBound) +
                                " " + std::to_string(kMaxModulus) + " " +
                                std::to_string(kMaxThreads) + " 0";
    EXPECT_TRUE(parseInput(largest).query) << largest;
}

// ---------------------------------------------------------------------------
// Rejected input: one test per rule
// ---------------------------------------------------------------------------

TEST(InputParser, RejectsWrongNumberOfFields) {
    expectError("", "expected 5 to 7");
    expectError("1 2 3 4", "expected 5 to 7");
    expectError("0 1 2 3 4 5 6 7", "expected 5 to 7");
}

TEST(InputParser, RejectsNonIntegers) {
    expectError("abc 20 11 4 0", "not an integer");
    expectError("12abc 20 11 4 0", "not an integer");
    expectError("1.5 20 11 4 0", "not an integer");
}

TEST(InputParser, RejectsNumbersBeyond64Bits) {
    expectError("0 99999999999999999999 11 4 0", "too large");
}

TEST(InputParser, RejectsNonZeroLeadingValue) {
    // Only 0 is accepted, so a shifted or mistyped line is not silently misread.
    expectError("1 274856170 274856190 11 4 0 0", "leading 0");
    expectError("5 274856170 274856190 11 4 0 0", "leading 0");
}

TEST(InputParser, RejectsInvalidLowerBound) {
    expectError("-1 20 11 4 0", "b must");
    expectError("2147483648 2147483649 11 4 0", "b must");  // 2^31
}

TEST(InputParser, RejectsInvalidUpperBound) {
    expectError("20 20 11 4 0", "e must");  // e must be greater than b
    expectError("20 10 11 4 0", "e must");
    expectError("0 2147483648 11 4 0", "e must");  // 2^31
}

TEST(InputParser, RejectsInvalidModulus) {
    expectError("0 20 0 4 0", "m must");
    expectError("0 20 257 4 0", "m must");
}

TEST(InputParser, RejectsInvalidThreadCount) {
    expectError("0 20 11 0 0", "p must");
    expectError("0 20 11 257 0", "p must");
}

TEST(InputParser, RejectsInvalidMode) {
    expectError("0 20 11 4 3", "u must");
    expectError("0 20 11 4 -1", "u must");
}

TEST(InputParser, SearchModeRequiresTarget) {
    expectError("0 20 11 4 2", "requires s");
}

// When several fields are wrong, the first one in input order is reported.
TEST(InputParser, ReportsFirstInvalidField) {
    expectError("-1 20 0 4 0", "b must");  // b and m are both wrong
}
