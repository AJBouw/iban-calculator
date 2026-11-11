/// @file MCheck.h
/// @brief The m-check: an account number is valid when its weighted digit sum
///        is divisible by m.
/// @note Both functions are thread-safe: they use only their arguments and
///       local variables, so concurrent calls share no mutable state.
// LEARN: A header only declares functions (name, parameters, return type).
// LEARN: The code itself (the definition) lives in src/domain/MCheck.cpp.
// LEARN: Headers in include/ are the public API; other targets may include them.

#pragma once
// LEARN: #pragma once: the compiler reads this file at most once per .cpp file,
// LEARN: even when it is included several times. Prevents double declarations.

#include <cstdint>
// LEARN: <cstdint> provides fixed-width integers such as std::int64_t.

// Domain layer: pure calculation, no input/output and no threads, so it can be
// tested in isolation and reused by every execution strategy.
// LEARN: A namespace groups names and prevents clashes with other libraries.
// LEARN: iban::domain mirrors the folder include/iban/domain/.
namespace iban::domain {

/// @brief Weighted digit sum: every digit times its position, counted from the
///        right and starting at 1. Example: 274856182 gives 220.
/// @param number Account number; must be >= 0. 0 is valid input and gives 0.
/// @return The sum: at most 495 for the numbers in this assignment (10 digits)
///         and at most 1710 for any positive std::int64_t (19 digits), so it
///         always fits in an int.
/// @throws std::invalid_argument when @p number is negative.
// LEARN: std::int64_t provides a 64-bit signed integer type.
// LEARN: It gives us sufficient headroom for range calculations and
// LEARN: intermediate results, provided calculations use this type.
// LEARN: [[nodiscard]]: the compiler warns when the caller ignores the result,
// LEARN: because calling this function without using the result is always a bug.
[[nodiscard]] int weightedDigitSum(std::int64_t number);

/// @brief True when the weighted digit sum of @p number is divisible by @p modulus.
/// @param number Account number; must be >= 0. 0 has sum 0 and therefore
///        passes for every modulus.
/// @param modulus The m from the assignment; must be > 0.
/// @throws std::invalid_argument when @p modulus <= 0 (x % 0 is undefined
///         behaviour) or @p number is negative.
// LEARN: Fail fast: wrong input throws immediately instead of giving a wrong answer.
// LEARN: The input parser also validates m, but this function is public,
// LEARN: so it protects itself and does not depend on its caller.
[[nodiscard]] bool passesMCheck(std::int64_t number, int modulus);

}  // namespace iban::domain
// LEARN: The comment after } is Google style: in long files you see what closes.
