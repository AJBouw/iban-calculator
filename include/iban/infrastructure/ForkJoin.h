/// @file ForkJoin.h
/// @brief Runs one piece of work per thread and waits for all of them: the
///        thread handling that every count variant shares.
// LEARN: Fork-join: start ("fork") several threads, then wait ("join") for all.

#pragma once

#include <cstddef>
#include <thread>
#include <vector>

namespace iban::infrastructure {

namespace detail {

// Waits for every thread that is still running.
// LEARN: A std::thread that is destroyed while still joinable calls
// LEARN: std::terminate(), so every started thread must be joined (or detached).
inline void joinAll(std::vector<std::thread>& threads) {
    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

}  // namespace detail

/// @brief Calls work(index) on a separate std::thread for every index in
///        [0, threadCount) and returns when all threads have finished.
///
/// std::thread with explicit joins, as in the lectures. std::jthread (C++20)
/// would join in its destructor (RAII) and offer a std::stop_token; see
/// docs/decisions/0007-threads-and-result-slots.md.
/// @param threadCount Number of threads to start.
/// @param work Called as work(std::size_t index); must not throw, because an
///        exception that leaves a thread ends the program (std::terminate).
/// @throws std::system_error when a thread cannot be started; the threads that
///         were already started are joined first.
// LEARN: work is copied into every thread; a lambda that captures by reference
// LEARN: copies only the references, so all threads use the same data.
template <typename Work>
void runInParallel(std::size_t threadCount, Work work) {
    std::vector<std::thread> threads;
    // LEARN: reserve first, so emplace_back never moves running thread objects.
    threads.reserve(threadCount);
    try {
        for (std::size_t index = 0; index < threadCount; ++index) {
            threads.emplace_back(work, index);
        }
    } catch (...) {
        // The threads already running use the caller's data, so they are joined
        // before the error is passed on.
        detail::joinAll(threads);
        throw;
    }
    // After join, every write of a thread is visible to the caller.
    detail::joinAll(threads);
}

}  // namespace iban::infrastructure
