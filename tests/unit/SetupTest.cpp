/// Smoke tests for the build environment.
/// When these pass on macOS, Linux and Windows (see CI), the compiler,
/// C++20 mode and threading work on every platform.
/// They stay in the suite to catch a broken toolchain early.

// LEARN: GoogleTest is a test framework.
// Every TEST(Group, Name) { ... } is a separate test;
// EXPECT_... verifies and notifies a mistake when the test fails.
#include <gtest/gtest.h>

#include <atomic>
#include <thread>

/// The compiler must run in C++20 mode (required by CMakeLists.txt).
TEST(Setup, CompilerUsesCpp20) {
    // LEARN: __cplusplus is a number the compiler fills in;
    // 202002L means C++20. static_assert checks during compile time:
    // if it's incorrect, the project will not be built.
    static_assert(__cplusplus >= 202002L, "C++20 required");
    SUCCEED();
}

/// Starting and joining threads works on this platform.
TEST(Setup, ThreadsWork) {
    // LEARN: std::atomic<int> is an int that can be safely modified by multiple threads at the same time.
    std::atomic<int> counter{0};

    // LEARN: lambda [&] { ... } is a piece of code like variabele;
    // [&] meaning: use the variables from outside via the reference.
    std::thread first([&] { counter.fetch_add(1); });
    std::thread second([&] { counter.fetch_add(1); });

    // LEARN: join() waits until the thread is ready.
    // An std::thread not being joined before it's cleaned up makes the program crash
    // (std::terminate). Here the RAII solution comes in.
    first.join();
    second.join();

    EXPECT_EQ(counter.load(), 2);
}