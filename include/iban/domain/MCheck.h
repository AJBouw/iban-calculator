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

}  // namespace iban::domain