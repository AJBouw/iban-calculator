/// @file Count.cpp
/// @brief Implementation of the sequential count. See Count.h.

#include "iban/domain/Count.h"

#include <stdexcept>

#include "iban/domain/MCheck.h"

namespace iban::domain {

std::int64_t countValid(Range range, int modulus) {
    // Checked once here, so an invalid modulus is reported before the loop
    // instead of on its first iteration (or never, for an empty range).
    if (modulus <= 0) {
        throw std::invalid_argument("modulus must be greater than 0");
    }

    std::int64_t count = 0;
    // LEARN: Half-open range: x starts at begin and stops before end.
    for (std::int64_t number = range.begin; number < range.end; ++number) {
        if (passesMCheck(number, modulus)) {
            ++count;
        }
    }
    return count;
}

}  // namespace iban::domain
