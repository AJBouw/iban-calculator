/// @file SharedCounter.h
/// @brief A counter that several threads add to, protected by a lock: the
///        shared counter that the assignment requires for count mode.

#pragma once

#include <cstdint>
#include <mutex>

#include "iban/infrastructure/BasicLockable.h"

namespace iban::infrastructure {

/// @brief Thread-safe counter; every access holds the lock.
/// @tparam Lock Any BasicLockable type, for example std::mutex (default) or
///         SpinLock. A template parameter, so lock() and unlock() can be inlined
///         (docs/decisions/0010-shared-counter-and-lock.md).
// LEARN: The lock lives next to the value it protects, so no caller can reach
// LEARN: the value without the lock.
template <BasicLockable Lock = std::mutex>
class SharedCounter {
public:
    /// Adds @p amount. Holds the lock only for the addition itself.
    void add(std::int64_t amount) {
        // RAII: the lock is released when guard goes out of scope, also when an
        // exception is thrown.
        // LEARN: std::lock_guard guard(lock_) deduces lock_guard<Lock> (C++17 CTAD).
        std::lock_guard guard(lock_);
        value_ += amount;
    }

    /// The current value. Taken under the lock, so a read never sees half of a
    /// write; after all threads have been joined it is the final result.
    [[nodiscard]] std::int64_t value() const {
        std::lock_guard guard(lock_);
        return value_;
    }

private:
    // mutable: value() is const for its callers but must still take the lock.
    mutable Lock lock_;
    std::int64_t value_ = 0;
};

}  // namespace iban::infrastructure
