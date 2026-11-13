/// @file SpinLock.h
/// @brief A lock that waits by checking a flag in a loop instead of sleeping,
///        to measure against std::mutex.
// LEARN: A mutex lets a waiting thread sleep (the operating system wakes it);
// LEARN: a spin lock keeps the thread running and checks again and again.

#pragma once

#include <atomic>
#include <thread>

namespace iban::infrastructure {

/// @brief Spin lock on std::atomic_flag; satisfies BasicLockable.
///
/// Suited to very short critical sections with little waiting. With more
/// threads than cores, a waiting thread spins on a core that the lock holder
/// needs, so the loop yields its time slice while the lock is taken.
class SpinLock {
public:
    /// Waits until the flag is clear and sets it.
    void lock() noexcept {
        // test_and_set sets the flag and returns its previous value: false
        // means this thread took the lock. acquire: the reads and writes of the
        // critical section cannot move before taking the lock.
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Wait with plain reads until the lock looks free (test and
            // test-and-set): reading does not take the cache line away from
            // the thread that holds the lock; writing would.
            // LEARN: atomic_flag::test (C++20) reads the flag without changing it.
            while (flag_.test(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
        }
    }

    /// Clears the flag. release: the writes of the critical section are
    /// visible to the next thread that takes the lock.
    void unlock() noexcept { flag_.clear(std::memory_order_release); }

private:
    // LEARN: Since C++20 a default-constructed atomic_flag is clear (unlocked).
    std::atomic_flag flag_;
};

}  // namespace iban::infrastructure
