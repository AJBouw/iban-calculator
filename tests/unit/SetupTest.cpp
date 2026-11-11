/// @file SetupTest.cpp
/// @brief Smoke tests for the build environment.
///        Verifies C++20 support and basic thread functionality.

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

// The compiler must run in C++20 mode (required by CMakeLists.txt).
TEST(Setup, CompilerUsesCpp20) {
    // LEARN: static_assert is checked during compilation.
    // LEARN: If the condition is false, compilation fails.
    // LEARN: __cplusplus is set by the compiler; 202002L represents C++20.
    static_assert(__cplusplus >= 202002L, "C++20 required");

    SUCCEED();
}

// Verify that threads can be started, synchronized and joined.
TEST(Setup, ThreadsWork) {
    // LEARN: std::atomic<int> can be safely modified by multiple threads.
    std::atomic<int> counter{0};

    // LEARN: A lambda is a callable object.
    // LEARN: [&] captures variables from the surrounding scope by reference.
    std::thread first([&] { counter.fetch_add(1); });
    std::thread second([&] { counter.fetch_add(1); });

    // LEARN: join() waits until the thread has finished.
    // LEARN: A joinable thread must be joined or detached before destruction.
    first.join();
    second.join();

    EXPECT_EQ(counter.load(), 2);
}