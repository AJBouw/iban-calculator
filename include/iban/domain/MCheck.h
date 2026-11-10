/// @file MCheck.h
/// @brief The m-check ("m-proef"): a generalisation of the Dutch 11-proef.
///
/// Every digit is multiplied by a weight based on its position from the
/// right (last digit x1, second-to-last x2, ...). A number passes the m-check
/// when that weighted sum is a multiple of the modulus m.
///
/// Domain layer: pure logic only. No threads, no I/O, no knowledge of how
/// the numbers are distributed or reported.

// LEARN: #pragma once makes sure this header is included only once per .cpp file,
// LEARN: even if several headers include it (prevents "redefinition" errors).
#pragma once

// LEARN: <cstdint> provides fixed-width integer types such as std::int64_t.
#include <cstdint>

// LEARN: a namespace groups names and prevents clashes with other code.
namespace iban::domain {

/// @brief Weighted digit sum: last digit x1, second-to-last x2, and so on.
///
/// Works for any number of digits (the assignment states numbers need not
/// have 9 digits).
///
/// @param number The number to process.
/// @return The weighted digit sum.
/// @pre `number >= 0`.
/// @throws std::invalid_argument if `number < 0`.
/// @par Complexity
/// O(d) time and O(1) space, where d is the number of digits.
int weightedDigitSum(std::int64_t number);

/// @brief Checks whether `number` passes the m-check.
///
/// @param number  The number to check.
/// @param modulus The modulus m.
/// @return `true` when the weighted digit sum of `number` is a multiple of `modulus`.
/// @pre `number >= 0` and `modulus > 0`.
/// @throws std::invalid_argument if a precondition is violated.
/// @note Open question (to be confirmed with the lecturer): 0 has digit sum 0,
///       which is a multiple of every modulus, so 0 passes. This function is
///       the only place to change if that is not intended.
/// @par Complexity
/// O(d) time and O(1) space, where d is the number of digits.
bool passesMCheck(std::int64_t number, int modulus);

/// @brief Walks through consecutive numbers and keeps the weighted digit sum
///        up to date incrementally, instead of recomputing it for every number.
///
/// Going from x to x+1 without a carry only changes the last digit, which has
/// weight 1, so the sum increases by exactly 1. Only when x ends in 9 (a
/// carry) is the sum recomputed with weightedDigitSum(), on average once
/// every 10 steps.
///
/// Design decision: this requires each thread to walk a *contiguous* range,
/// so the range is split into consecutive blocks rather than interleaved.
///
/// @par Complexity
/// Amortised O(1) per step instead of O(d): one in ten steps costs O(d),
/// the other nine cost O(1). Measured on the full range [0, 2^31-1) with one
/// thread, this is about 3.5x faster than calling passesMCheck() per number.
///
/// @par Example
/// @code
/// for (MCheckWalker walker(begin, modulus); walker.value() < end; walker.advance()) {
///     if (walker.isValid()) { /* ... */ }
/// }
/// @endcode
// LEARN: a class bundles data (the fields) with the functions that work on that data.
class MCheckWalker {
public:
    /// @brief Starts walking at `start`.
    /// @param start   First number of the walk.
    /// @param modulus The modulus m.
    /// @pre `start >= 0` and `modulus > 0`.
    /// @throws std::invalid_argument if a precondition is violated.
    MCheckWalker(std::int64_t start, int modulus);

    // LEARN: `const` after a member function means: this function does not change the object.
    // LEARN: `noexcept` promises this function never throws an exception.

    /// @brief The current number.
    std::int64_t value() const noexcept { return current_; }

    /// @brief Weighted digit sum of the current number.
    int digitSum() const noexcept { return sum_; }

    /// @brief `true` when the current number passes the m-check.
    bool isValid() const noexcept { return sum_ % modulus_ == 0; }

    /// @brief Moves to the next number (current + 1).
    void advance();

    // LEARN: private = only the class itself can access these fields.
    // LEARN: the trailing underscore is a naming convention for member variables.
private:
    std::int64_t current_;
    int modulus_;
    int sum_;
};

}  // namespace iban::domain