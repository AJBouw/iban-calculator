/// @file BoundedBuffer.h
/// @brief A queue with a maximum size for producer threads and consumer threads:
///        the bounded buffer from the lectures, built from a mutex and two
///        condition variables.

#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

namespace iban::infrastructure {

/// @brief Thread-safe FIFO queue of at most capacity() items.
///
/// push() waits while the buffer is full and pop() waits while it is empty, so
/// a fast producer cannot fill the memory and a consumer never polls in a loop.
/// close() ends the work: pop() then returns the remaining items and after
/// that std::nullopt, so a consumer knows that nothing more will come.
// LEARN: A condition variable lets a thread sleep until another thread changes
// LEARN: the shared state and notifies it; it is always used with a mutex.
template <typename T>
class BoundedBuffer {
public:
    /// @throws std::invalid_argument when @p capacity is 0.
    explicit BoundedBuffer(std::size_t capacity) : capacity_(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("capacity must be greater than 0");
        }
    }

    /// Adds @p value; waits while the buffer is full.
    /// @return false when the buffer was closed; the value is then dropped.
    [[nodiscard]] bool push(T value) {
        std::unique_lock lock(mutex_);
        // LEARN: wait(lock, predicate) is the lectures' "while (!cond) cv.wait(l)":
        // LEARN: it checks again after every wake-up, also a spurious one.
        notFull_.wait(lock, [this] { return items_.size() < capacity_ || closed_; });
        if (closed_) {
            return false;
        }
        items_.push_back(std::move(value));
        // Notified after unlocking, so the woken consumer does not wait for the
        // lock that this thread still holds.
        lock.unlock();
        notEmpty_.notify_one();
        return true;
    }

    /// Takes the oldest item; waits while the buffer is empty and open.
    /// @return std::nullopt when the buffer is closed and empty.
    [[nodiscard]] std::optional<T> pop() {
        std::unique_lock lock(mutex_);
        notEmpty_.wait(lock, [this] { return !items_.empty() || closed_; });
        if (items_.empty()) {
            return std::nullopt;  // closed, and every item has been taken
        }
        T value = std::move(items_.front());
        items_.pop_front();
        lock.unlock();
        notFull_.notify_one();
        return value;
    }

    /// No more items will be pushed. Wakes every waiting thread.
    void close() {
        {
            std::lock_guard guard(mutex_);
            closed_ = true;
        }
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

private:
    const std::size_t capacity_;
    std::mutex mutex_;
    // LEARN: Two condition variables: producers wait for "not full", consumers
    // LEARN: for "not empty", so a notification wakes the right kind of thread.
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    std::deque<T> items_;
    bool closed_ = false;
};

}  // namespace iban::infrastructure
