/// @file ConsoleInputTest.cpp
/// @brief Unit tests for the console input: one value per question, a pasted
///        complete line, skipping s, repeated questions and end of input.
// LEARN: std::istringstream acts as the keyboard and std::ostringstream as the
// LEARN: screen, so these tests need no real console.

#include <gtest/gtest.h>

#include <optional>
#include <sstream>
#include <string>

#include "iban/presentation/ConsoleInput.h"

using iban::presentation::readInputLine;

namespace {

// Runs readInputLine with `typed` as the keyboard input.
std::optional<std::string> readFrom(const std::string& typed) {
    std::istringstream keyboard(typed);
    std::ostringstream screen;
    return readInputLine(keyboard, screen);
}

}  // namespace

TEST(ConsoleInput, JoinsOneValuePerQuestion) {
    EXPECT_EQ(readFrom("274856170\n274856190\n11\n4\n2\n274856182\n"),
              "274856170 274856190 11 4 2 274856182");
}

TEST(ConsoleInput, AcceptsCompleteLineAtFirstQuestion) {
    // The 7-number example from the assignment, pasted or piped as one line.
    EXPECT_EQ(readFrom("0 274856170 274856190 11 4 0 0\n"), "0 274856170 274856190 11 4 0 0");
}

TEST(ConsoleInput, SkipsEmptySearchValue) {
    EXPECT_EQ(readFrom("10\n20\n7\n2\n0\n\n"), "10 20 7 2 0");
}

TEST(ConsoleInput, AsksAgainAfterEmptyRequiredValue) {
    EXPECT_EQ(readFrom("\n   \n10\n20\n7\n2\n0\n\n"), "10 20 7 2 0");
}

TEST(ConsoleInput, ReturnsNothingWithoutInput) {
    EXPECT_FALSE(readFrom("").has_value());
}

TEST(ConsoleInput, ReturnsPartialInputSoTheParserCanReportIt) {
    // Input ends after two values; the parser then reports the missing fields.
    EXPECT_EQ(readFrom("10\n20\n"), "10 20");
}

TEST(ConsoleInput, ExplainsEveryField) {
    std::istringstream keyboard("1\n2\n3\n4\n0\n\n");
    std::ostringstream screen;
    (void)readInputLine(keyboard, screen);
    const std::string shown = screen.str();
    for (const char* letter : {"  b  ", "  e  ", "  m  ", "  p  ", "  u  ", "  s  "}) {
        EXPECT_NE(shown.find(letter), std::string::npos) << "missing field" << letter;
    }
    EXPECT_NE(shown.find("1 to 256"), std::string::npos);  // ranges from Limits.h
    EXPECT_NE(shown.find("extra 0"), std::string::npos);   // the leading-zero hint
}
