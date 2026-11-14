/// @file SequencedWriterTest.cpp
/// @brief Unit tests for the sequenced writer: with many threads writing at
///        once, the sequence numbers still ascend line by line.

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <string>

#include "iban/infrastructure/ForkJoin.h"
#include "iban/infrastructure/SequencedWriter.h"
#include "iban/infrastructure/SpinLock.h"

using iban::infrastructure::runInParallel;
using iban::infrastructure::SequencedWriter;
using iban::infrastructure::SpinLock;

template <typename Lock>
class SequencedWriterTest : public ::testing::Test {};

using LockTypes = ::testing::Types<std::mutex, SpinLock>;
TYPED_TEST_SUITE(SequencedWriterTest, LockTypes);

TYPED_TEST(SequencedWriterTest, WritesNumberedLines) {
    std::ostringstream out;
    SequencedWriter<TypeParam> writer(out);
    writer.write(274856182);
    writer.write(274856174);
    EXPECT_EQ(out.str(), "1 274856182\n2 274856174\n");
    EXPECT_EQ(writer.written(), 2);
}

// Without one critical section for the sequence number and the line, a line
// with a higher number could be written before a lower one, or two lines could
// be mixed. Every thread writes its own index, so the lines can be checked.
TYPED_TEST(SequencedWriterTest, SequenceNumbersAscendWithManyThreads) {
    constexpr std::size_t kThreads = 16;
    constexpr int kLinesPerThread = 2'000;
    std::ostringstream out;
    SequencedWriter<TypeParam> writer(out);

    runInParallel(kThreads, [&writer](std::size_t index) {
        for (int line = 0; line < kLinesPerThread; ++line) {
            writer.write(static_cast<std::int64_t>(index));
        }
    });

    std::istringstream lines(out.str());
    std::string line;
    std::int64_t expected = 1;
    while (std::getline(lines, line)) {
        std::istringstream fields(line);
        std::int64_t sequence = 0;
        std::int64_t number = -1;
        ASSERT_TRUE(fields >> sequence >> number) << "line '" << line << "'";
        ASSERT_EQ(sequence, expected) << "line '" << line << "'";
        ASSERT_GE(number, 0);
        ASSERT_LT(number, static_cast<std::int64_t>(kThreads));
        ++expected;
    }
    EXPECT_EQ(expected - 1, static_cast<std::int64_t>(kThreads) * kLinesPerThread);
    EXPECT_EQ(writer.written(), static_cast<std::int64_t>(kThreads) * kLinesPerThread);
}
