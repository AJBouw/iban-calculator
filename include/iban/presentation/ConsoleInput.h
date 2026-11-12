/// @file ConsoleInput.h
/// @brief Asks the user for the input values and returns them as one line for
///        parseInput. Validation is left to the parser, so it lives in one place.
// LEARN: The streams are parameters instead of std::cin/std::cerr directly, so
// LEARN: the tests can pass a std::istringstream and check the prompts.

#pragma once

#include <iosfwd>
#include <optional>
#include <string>

namespace iban::presentation {

/// @brief Reads the input values from @p in, asking for them on @p prompt.
///
/// The user is asked for b, e, m, p, u and s one at a time; an empty answer
/// for s skips it. When the first answer contains more than one value, it is
/// taken as the complete input line, so a whole line can still be pasted or
/// piped, including the 7-number examples from the assignment.
/// @return The values as one space-separated line, or nothing at end of input.
// LEARN: <iosfwd> only declares the stream types; the full <istream> and
// LEARN: <ostream> are included in the .cpp, which keeps this header light.
[[nodiscard]] std::optional<std::string> readInputLine(std::istream& in, std::ostream& prompt);

}  // namespace iban::presentation
