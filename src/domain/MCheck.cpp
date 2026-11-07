/// @file MCheck.cpp
/// @brief Implementation of the m-check. See MCheck.h for the design notes.

// LEARN: a .cpp file includes its own header first; that verifies the header
// LEARN: compiles on its own (it includes everything it needs).
#include "iban/domain/MCheck.h"

#include <stdexcept>

namespace iban::domain {

// Internal implementation details of this file. The anonymous namespace gives
// them internal linkage: two .cpp files defining a helper with the same name
// would otherwise cause a link error ("duplicate symbol"), or for classes and
// inline functions a silent One Definition Rule violation.
// constexpr variables already have internal linkage; it is used here for
// consistency, so every helper added later is internal as well.
// LEARN: an anonymous namespace makes these names visible in this .cpp file only.
namespace {

// Base of the number system; a named constant instead of the magic number 10.
// LEARN: constexpr = a constant whose value is known at compile time;
// LEARN: the type-safe replacement for #define (typed, scoped, visible in the debugger).
constexpr int kDecimalBase = 10;

}  // namespace

int weightedDigitSum(std::int64_t number) {
    if (number < 0) {
        // LEARN: throw stops the function and hands an error to the caller.
        throw std::invalid_argument("weightedDigitSum: number must be >= 0");
    }

    int sum = 0;
    int weight = 1;
    while (number > 0) {
        // x % 10 is the last digit, x / 10 removes it. The cast is safe: the
        // remainder is always 0..9. It is explicit because -Wconversion (see
        // iban_set_warnings in CMakeLists.txt) warns about implicit narrowing.
        // LEARN: static_cast makes a type conversion explicit and intentional.
        sum += static_cast<int>(number % kDecimalBase) * weight;
        number /= kDecimalBase;
        ++weight;
    }
    return sum;
}

bool passesMCheck(std::int64_t number, int modulus) {
    // x % 0 is undefined behaviour in C++ (it may crash or return anything),
    // so a non-positive modulus is rejected explicitly (fail fast).
    if (modulus <= 0) {
        throw std::invalid_argument("passesMCheck: modulus must be > 0");
    }
    return weightedDigitSum(number) % modulus == 0;
}

}  // namespace iban::domain