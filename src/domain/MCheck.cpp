
/// @file MCheck.cpp
/// @brief Reference implementation of the m-check. Kept simple and obviously
///        correct, because faster variants are tested against it.
// LEARN: A .cpp file contains the definitions: the actual code of the functions.
// LEARN: It is compiled once into an object file; the linker joins them later.

#include "iban/domain/MCheck.h"
// LEARN: Own header first (Google style): proves the header compiles on its own,
// LEARN: without depending on includes that happen to come before it.

#include <stdexcept>
// LEARN: <stdexcept> provides std::invalid_argument.

namespace iban::domain {

// LEARN: Each call has its own local variables, so concurrent calls do not
// LEARN: access or modify shared mutable state.

// LEARN: An anonymous namespace makes its contents visible in this file only
// LEARN: (internal linkage), so the name cannot clash with other files.
namespace {

// Base of the decimal system: number % kBase is the last digit and
// number / kBase removes it.
// LEARN: constexpr instead of #define: it has a type, a scope and shows in the
// LEARN: debugger. A #define is plain text replacement without any of these.
constexpr std::int64_t kBase = 10;

}  // namespace

int weightedDigitSum(std::int64_t number) {
    // Reject invalid input at the public function boundary.
    if (number < 0) {
        throw std::invalid_argument("number must not be negative");
    }

    // Arithmetic instead of converting to text: no memory allocation and
    // O(d) steps for a number with d digits.
    // LEARN: number is a copy (passed by value), so changing it is safe here.
    int sum = 0;
    int weight = 1;  // the rightmost digit has weight 1

    while (number > 0) {
        // LEARN: static_cast makes the narrowing from int64 to int explicit;
        // LEARN: -Wconversion would warn otherwise. A digit is 0-9, so it fits.
        const auto digit = static_cast<int>(number % kBase);

        sum += weight * digit;
        number /= kBase;
        ++weight;
    }

    // LEARN: number 0 skips the loop and returns 0, which every positive m divides.
    return sum;
}
// LEARN: Complexity:
// LEARN: - Time: O(d), where d is the number of decimal digits.
// LEARN: - Space: O(1), using a fixed number of variables.

bool passesMCheck(std::int64_t number, int modulus) {
    // x % 0 is undefined behaviour, so an invalid modulus is rejected before use.
    if (modulus <= 0) {
        throw std::invalid_argument("modulus must be greater than 0");
    }

    return weightedDigitSum(number) % modulus == 0;
}
// LEARN: Complexity:
// LEARN: - Time: O(d), because weightedDigitSum processes all decimal digits.
// LEARN: - Space: O(1), because weightedDigitSum uses only a fixed number of variables.

}  // namespace iban::domain
