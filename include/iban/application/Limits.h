/// @file Limits.h
/// @brief Input rules from the assignment for b e m p u s, in one place, so the
///        parser, the Query defaults, and the tests all use the same values.
///        The mode u is limited by the Mode enum in Mode.h; s has no bounds.
// LEARN: Only constants, no functions: this header needs no .cpp file.
// LEARN: Application layer, not domain: the m-check itself works for any m > 0;
// LEARN: these numbers are rules of the assignment, not of the calculation.
// LEARN: Names are written out (kMinBound, not kMinB): b only means something
// LEARN: next to the assignment; the letter is kept in the comments instead.
// LEARN: The k prefix is the Google style rule for constants, so they stand out
// LEARN: from variables; ALL_CAPS is reserved for #define macros.

#pragma once

#include <cstdint>

namespace iban::application {

// LEARN: inline constexpr: a compile-time constant that may appear in many .cpp
// LEARN: files without breaking the one-definition rule (C++17 and later).

/// Smallest allowed lower bound b (assignment: 0 <= b).
inline constexpr std::int64_t kMinBound = 0;

/// Largest allowed bound for b and e (assignment: b, e <= 2^31 - 1), the largest
/// value of a 32-bit signed integer.
// LEARN: Written as a calculation, so the reader sees where the number comes from.
// LEARN: std::int64_t{1} makes the shift 64-bit; 1 << 31 on a 32-bit int overflows.
inline constexpr std::int64_t kMaxBound = (std::int64_t{1} << 31) - 1;

/// Smallest and largest allowed modulus m (assignment: 1 <= m <= 256).
inline constexpr int kMinModulus = 1;
inline constexpr int kMaxModulus = 256;

/// Smallest and largest allowed thread count p (assignment: 1 <= p <= 256).
inline constexpr int kMinThreads = 1;
inline constexpr int kMaxThreads = 256;

// Compile-time checks: a wrong edit of the constants fails the build.
// LEARN: static_assert is checked by the compiler; it costs nothing at run time.
static_assert(kMaxBound == 2'147'483'647, "the assignment bound is 2^31 - 1");
static_assert(kMinModulus > 0, "x % 0 is undefined behaviour");
static_assert(kMinThreads > 0, "at least one thread must do the work");

}  // namespace iban::application
