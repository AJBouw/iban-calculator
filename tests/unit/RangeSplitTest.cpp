/// @file RangeSplitTest.cpp
/// @brief Unit tests for the range split: examples, the case of more parts than
///        values, and the three requirements checked for many combinations.
// LEARN: Besides examples, these tests check properties (no gaps, no overlap,
// LEARN: fair sizes) for hundreds of inputs: a property must hold for every one.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "iban/application/Limits.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"

using iban::domain::Range;
using iban::domain::splitRange;

namespace {

// Checks the three requirements of the assignment for one split.
void expectFairSplit(Range range, int parts) {
    const std::vector<Range> split = splitRange(range, parts);
    ASSERT_EQ(split.size(), static_cast<std::size_t>(parts));

    // No gaps and no overlap: the parts are adjacent and cover the whole range.
    EXPECT_EQ(split.front().begin, range.begin);
    EXPECT_EQ(split.back().end, range.end);
    for (std::size_t i = 0; i + 1 < split.size(); ++i) {
        EXPECT_EQ(split[i].end, split[i + 1].begin) << "between part " << i << " and " << i + 1;
    }

    // Fair: the largest and smallest part differ by at most one.
    // LEARN: minmax_element returns iterators to the smallest and largest part.
    const auto [smallest, largest] = std::minmax_element(
        split.begin(), split.end(),
        [](const Range& left, const Range& right) { return left.size() < right.size(); });
    EXPECT_LE(largest->size() - smallest->size(), 1)
        << "range [" << range.begin << ", " << range.end << ") into " << parts;
}

}  // namespace

// ---------------------------------------------------------------------------
// Examples
// ---------------------------------------------------------------------------

TEST(RangeSplit, SplitsAssignmentExampleEvenly) {
    // 20 numbers over 4 threads: 5 each.
    const std::vector<Range> expected{{274856170, 274856175},
                                      {274856175, 274856180},
                                      {274856180, 274856185},
                                      {274856185, 274856190}};
    EXPECT_EQ(splitRange(Range{274856170, 274856190}, 4), expected);
}

TEST(RangeSplit, GivesTheRemainderToTheFirstParts) {
    // 10 numbers over 4 parts: q = 2, r = 2, so sizes 3, 3, 2, 2.
    const std::vector<Range> expected{{0, 3}, {3, 6}, {6, 8}, {8, 10}};
    EXPECT_EQ(splitRange(Range{0, 10}, 4), expected);
}

TEST(RangeSplit, OnePartIsTheWholeRange) {
    // LEARN: Commas inside a macro argument such as EXPECT_EQ confuse the
    // LEARN: preprocessor, so expected values go into a named variable first.
    const std::vector<Range> expected{{10, 20}};
    EXPECT_EQ(splitRange(Range{10, 20}, 1), expected);
}

// ---------------------------------------------------------------------------
// More parts than values: every thread still gets a part
// ---------------------------------------------------------------------------

TEST(RangeSplit, MorePartsThanValuesGivesEmptyLastParts) {
    // 2 numbers over 4 threads: two parts of one, then two empty parts.
    const std::vector<Range> expected{{0, 1}, {1, 2}, {2, 2}, {2, 2}};
    EXPECT_EQ(splitRange(Range{0, 2}, 4), expected);
}

TEST(RangeSplit, OneValueOverMaximumThreads) {
    const auto split = splitRange(Range{7, 8}, iban::application::kMaxThreads);
    ASSERT_EQ(split.size(), static_cast<std::size_t>(iban::application::kMaxThreads));
    EXPECT_EQ(split.front(), (Range{7, 8}));
    // LEARN: all_of checks a condition for every element after the first.
    EXPECT_TRUE(std::all_of(split.begin() + 1, split.end(),
                            [](const Range& part) { return part.empty(); }));
}

TEST(RangeSplit, EmptyRangeGivesEmptyParts) {
    for (const Range& part : splitRange(Range{5, 5}, 3)) {
        EXPECT_TRUE(part.empty());
    }
}

// ---------------------------------------------------------------------------
// Properties for many combinations
// ---------------------------------------------------------------------------

TEST(RangeSplit, IsFairForManySizesAndPartCounts) {
    for (std::int64_t size = 0; size <= 300; ++size) {
        for (int parts = 1; parts <= 20; ++parts) {
            expectFairSplit(Range{1000, 1000 + size}, parts);
        }
    }
}

TEST(RangeSplit, IsFairForTheLargestRange) {
    // The whole input range from the assignment, with the extreme thread counts.
    const Range largest{iban::application::kMinBound, iban::application::kMaxBound};
    for (const int parts : {1, 2, 3, 7, 255, iban::application::kMaxThreads}) {
        expectFairSplit(largest, parts);
    }
}

// ---------------------------------------------------------------------------
// Invalid arguments: programming errors, so they throw
// ---------------------------------------------------------------------------

TEST(RangeSplit, RejectsFewerThanOnePart) {
    EXPECT_THROW((void)splitRange(Range{0, 10}, 0), std::invalid_argument);
    EXPECT_THROW((void)splitRange(Range{0, 10}, -1), std::invalid_argument);
}

TEST(RangeSplit, RejectsReversedRange) {
    EXPECT_THROW((void)splitRange(Range{10, 0}, 2), std::invalid_argument);
}
