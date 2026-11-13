/// @file BasicLockable.h
/// @brief The concept for a lock that can be locked and unlocked, used to pass
///        the lock type to SharedCounter as a template parameter.
///
/// A template parameter instead of a virtual interface: the lock is known at
/// compile time, so lock() and unlock() can be inlined. The alternatives are in
/// docs/decisions/0010-shared-counter-and-lock.md.
// LEARN: A concept (C++20) names the requirements on a template parameter.
// LEARN: A type that misses lock() or unlock() gives one clear compiler error.

#pragma once

namespace iban::infrastructure {

/// Satisfied by every type with lock() and unlock(), such as std::mutex and
/// SpinLock; the same requirements as the standard library's BasicLockable.
// LEARN: requires(T& lockable) { ... } lists expressions that must compile.
template <typename T>
concept BasicLockable = requires(T& lockable) {
    lockable.lock();
    lockable.unlock();
};

}  // namespace iban::infrastructure
