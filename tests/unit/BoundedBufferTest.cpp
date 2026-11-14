/// @file BoundedBufferTest.cpp
/// @brief Unit tests for the bounded buffer: order, closing, and many producers
///        and consumers at once without losing or duplicating an item.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

#include "iban/infrastructure/BoundedBuffer.h"
#include "iban/infrastructure/ForkJoin.h"

using iban::infrastructure::BoundedBuffer;
using iban::infrastructure::runInParallel;

TEST(BoundedBuffer, ReturnsItemsInTheOrderTheyWerePushed) {
    BoundedBuffer<int> buffer(4);
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 3);
}

TEST(BoundedBuffer, GivesRemainingItemsAfterCloseAndThenNothing) {
    BoundedBuffer<int> buffer(4);
    EXPECT_TRUE(buffer.push(7));
    buffer.close();
    EXPECT_FALSE(buffer.push(8));  // closed: no new items
    EXPECT_EQ(buffer.pop(), 7);    // but the waiting item is still delivered
    EXPECT_EQ(buffer.pop(), std::nullopt);
}

TEST(BoundedBuffer, CloseWakesAWaitingConsumer) {
    BoundedBuffer<int> buffer(1);
    std::optional<int> result = 0;
    std::thread consumer([&buffer, &result] { result = buffer.pop(); });
    buffer.close();
    consumer.join();
    EXPECT_EQ(result, std::nullopt);
}

TEST(BoundedBuffer, RejectsCapacityZero) {
    EXPECT_THROW(BoundedBuffer<int>(0), std::invalid_argument);
}

// Capacity 1 forces producers to wait on almost every push: if a wake-up were
// lost, this test would hang (CTest stops it after its time-out).
TEST(BoundedBuffer, DeliversEveryItemOnceWithManyProducersAndConsumers) {
    constexpr std::size_t kProducers = 8;
    constexpr std::size_t kConsumers = 4;
    constexpr int kItemsPerProducer = 5'000;
    BoundedBuffer<int> buffer(1);

    std::mutex receivedLock;
    std::vector<int> received;
    std::vector<std::thread> consumers;
    for (std::size_t index = 0; index < kConsumers; ++index) {
        consumers.emplace_back([&buffer, &receivedLock, &received] {
            while (const std::optional<int> item = buffer.pop()) {
                std::lock_guard guard(receivedLock);
                received.push_back(*item);
            }
        });
    }
    runInParallel(kProducers, [&buffer](std::size_t producer) {
        for (int item = 0; item < kItemsPerProducer; ++item) {
            EXPECT_TRUE(buffer.push(static_cast<int>(producer) * kItemsPerProducer + item));
        }
    });
    buffer.close();
    for (std::thread& consumer : consumers) {
        consumer.join();
    }

    std::vector<int> expected(kProducers * kItemsPerProducer);
    std::iota(expected.begin(), expected.end(), 0);
    std::sort(received.begin(), received.end());
    EXPECT_EQ(received, expected);
}
