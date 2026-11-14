/// @file ForEachValid.h
/// @brief Calls a function for every number in a range that passes the
///        m-check, on the calling thread. List mode uses it to write every
///        valid number as soon as it is found.
// LEARN: Like countValid, but instead of counting it hands every valid number to
// LEARN: the caller, who decides what to do with it (write it, queue it, ...).

#pragma once

#include <cstdint>
#include <stdexcept>

#include "iban/domain/MCheck.h"
#include "iban/domain/Range.h"

namespace iban::domain {

/// @brief Calls visit(x) for every x in @p range with passesMCheck(x, modulus),
///        in ascending order.
/// @tparam Visit Callable as visit(std::int64_t).
/// @param range Half-open range of non-negative numbers; may be empty.
/// @param modulus Must be > 0.
/// @throws std::invalid_argument when @p modulus <= 0 or the range is negative.
// LEARN: A template, so the call to visit can be inlined; a std::function would
// LEARN: cost an indirect call for every valid number.
template <typename Visit>
void forEachValid(Range range, int modulus, Visit&& visit) {
    // Checked once, as in countValid, so an empty range still reports it.
    if (modulus <= 0) {
        throw std::invalid_argument("modulus must be greater than 0");
    }
    for (std::int64_t number = range.begin; number < range.end; ++number) {
        if (passesMCheck(number, modulus)) {
            visit(number);
        }
    }
}

}  // namespace iban::domain
