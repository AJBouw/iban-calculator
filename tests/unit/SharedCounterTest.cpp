/// @file SharedCounterTest.cpp
/// @brief Unit tests for the shared counter and the locks: many threads add at
///        the same time and no addition may be lost.
// LEARN: TYPED_TEST runs the same test body once for every type in a list,
// LEARN: here once with std::mutex and once with SpinLock.

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <mutex>

#include "iban/infrastructure/BasicLockable.h"
#include "iban/infrastructure/ForkJoin.h"
#include "iban/infrastructure/SharedCounter.h"
#include "iban/infrastructure/SpinLock.h"

using iban::infrastructure::BasicLockable;
using iban::infrastructure::runInParallel;
using iban::infrastructure::SharedCounter;
using iban::infrastructure::SpinLock;

// The concept accepts the locks the counter is used with and rejects a type
// without lock() and unlock(). Checked by the compiler, so no test is needed.
static_assert(BasicLockable<std::mutex>);
static_assert(BasicLockable<SpinLock>);
static_assert(!BasicLockable<int>);

template <typename Lock>
class SharedCounterTest : public ::testing::Test {};

using LockTypes = ::testing::Types<std::mutex, SpinLock>;
TYPED_TEST_SUITE(SharedCounterTest, LockTypes);

TYPED_TEST(SharedCounterTest, StartsAtZero) {
    const SharedCounter<TypeParam> counter;
    EXPECT_EQ(counter.value(), 0);
}

TYPED_TEST(SharedCounterTest, AddsAmounts) {
    SharedCounter<TypeParam> counter;
    counter.add(2);
    counter.add(40);
    EXPECT_EQ(counter.value(), 42);
}

// Without a lock, increments from different threads get lost (see
// tests/race/RaceDemo.cpp); with the lock, every one of them must count.
// More threads than most computers have cores, so threads also wait while the
// lock holder is not running, which matters for the spin lock.
TYPED_TEST(SharedCounterTest, LosesNoAdditionUnderContention) {
    constexpr std::size_t kThreads = 16;
    constexpr std::int64_t kAddsPerThread = 20'000;
    SharedCounter<TypeParam> counter;

    runInParallel(kThreads, [&counter](std::size_t /*index*/) {
        for (std::int64_t add = 0; add < kAddsPerThread; ++add) {
            counter.add(1);
        }
    });
    EXPECT_EQ(counter.value(), static_cast<std::int64_t>(kThreads) * kAddsPerThread);
}

TEST(RunInParallel, RunsEveryIndexExactlyOnce) {
    constexpr std::size_t kThreads = 32;
    SharedCounter<std::mutex> sumOfIndices;
    runInParallel(kThreads, [&sumOfIndices](std::size_t index) {
        sumOfIndices.add(static_cast<std::int64_t>(index));
    });
    // 0 + 1 + ... + 31 = 31 * 32 / 2
    EXPECT_EQ(sumOfIndices.value(), 496);
}
