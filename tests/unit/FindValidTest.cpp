/// @file FindValidTest.cpp
/// @brief Unit tests for findValid: it returns the first valid number that
///        matches the condition, looks only at valid numbers, and stops as
///        soon as it is asked to.

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "iban/domain/FindValid.h"
#include "iban/domain/Range.h"

using iban::domain::findValid;
using iban::domain::Range;

namespace {

// The range of the assignment example: 274856174 and 274856182 pass the 11-check.
constexpr Range kExample{274856170, 274856190};
constexpr int kModulus = 11;

const auto kAny = [](std::int64_t /*number*/) { return true; };
const auto kNeverStop = [] { return false; };

}  // namespace

TEST(FindValid, ReturnsTheFirstValidNumber) {
    EXPECT_EQ(findValid(kExample, kModulus, kAny, kNeverStop), 274856174);
}

TEST(FindValid, ReturnsTheValidNumberThatMatches) {
    const auto isTarget = [](std::int64_t number) { return number == 274856182; };
    EXPECT_EQ(findValid(kExample, kModulus, isTarget, kNeverStop), 274856182);
}

TEST(FindValid, DoesNotFindANumberThatFailsTheMCheck) {
    const auto isTarget = [](std::int64_t number) { return number == 274856183; };
    EXPECT_EQ(findValid(kExample, kModulus, isTarget, kNeverStop), std::nullopt);
}

// The condition is asked only about valid numbers, in ascending order.
TEST(FindValid, AsksTheConditionOnlyAboutValidNumbers) {
    std::vector<std::int64_t> asked;
    const auto record = [&asked](std::int64_t number) {
        asked.push_back(number);
        return false;
    };
    EXPECT_EQ(findValid(kExample, kModulus, record, kNeverStop), std::nullopt);
    EXPECT_EQ(asked, (std::vector<std::int64_t>{274856174, 274856182}));
}

// stopRequested is asked before every number, up to and including the one found.
TEST(FindValid, AsksWhetherToStopBeforeEveryNumber) {
    int asked = 0;
    const auto count = [&asked] {
        ++asked;
        return false;
    };
    const auto isTarget = [](std::int64_t number) { return number == 274856182; };
    EXPECT_EQ(findValid(kExample, kModulus, isTarget, count), 274856182);
    EXPECT_EQ(asked, 13);  // 274856170 up to and including 274856182
}

TEST(FindValid, StopsWhenAsked) {
    bool conditionAsked = false;
    const auto record = [&conditionAsked](std::int64_t /*number*/) {
        conditionAsked = true;
        return true;
    };
    EXPECT_EQ(findValid(kExample, kModulus, record, [] { return true; }), std::nullopt);
    EXPECT_FALSE(conditionAsked);
}

TEST(FindValid, EmptyRangeFindsNothing) {
    int asked = 0;
    const auto count = [&asked] {
        ++asked;
        return false;
    };
    EXPECT_EQ(findValid(Range{5, 5}, 1, kAny, count), std::nullopt);
    EXPECT_EQ(asked, 0);
}

TEST(FindValid, RejectsModulusBelowOne) {
    EXPECT_THROW(static_cast<void>(findValid(kExample, 0, kAny, kNeverStop)),
                 std::invalid_argument);
}
