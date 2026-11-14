/// @file SequencedWriter.h
/// @brief Writes numbers as numbered lines ("1 274856182") from several threads:
///        the sequence number and the line are one critical section, so the
///        numbers ascend in the order in which the lines are written.

#pragma once

#include <cstdint>
#include <mutex>
#include <ostream>
#include <string>

#include "iban/infrastructure/BasicLockable.h"

namespace iban::infrastructure {

/// @brief Thread-safe writer of numbered lines; satisfies the list mode of the
///        assignment: every number is written as soon as it is given.
/// @tparam Lock Any BasicLockable type, for example std::mutex (default) or
///         SpinLock (docs/decisions/0012-list-mode-output.md).
// LEARN: Taking the next sequence number and writing the line must happen
// LEARN: under the same lock. With two separate steps, thread A can take number
// LEARN: 3, thread B number 4, and B can write first: 4 before 3.
template <BasicLockable Lock = std::mutex>
class SequencedWriter {
public:
    /// @param out Stream for the lines; must outlive the writer. Only this
    ///        writer may use it while threads write.
    explicit SequencedWriter(std::ostream& out) : out_(out) {}

    /// Writes "<sequence number> <number>" and a line break.
    void write(std::int64_t number) {
        // The number is converted to text before the lock is taken, so the
        // lock is held only for the sequence number and one write.
        const std::string rest = ' ' + std::to_string(number) + '\n';
        std::lock_guard guard(lock_);
        ++written_;
        out_ << written_ << rest;
    }

    /// Number of lines written so far; read after the threads have been joined.
    [[nodiscard]] std::int64_t written() const {
        std::lock_guard guard(lock_);
        return written_;
    }

private:
    std::ostream& out_;
    // mutable: written() is const for its callers but must still take the lock.
    mutable Lock lock_;
    std::int64_t written_ = 0;
};

}  // namespace iban::infrastructure
