/// @file InputParser.h
/// @brief Turns one line of console input into a validated Query, or into an
///        error message that names the first invalid field.
// LEARN: Presentation layer: everything about text from and to the user lives
// LEARN: here. The layers below only see the typed, validated Query.

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "iban/application/Query.h"

namespace iban::presentation {

/// Result of parsing: exactly one of the two fields is filled.
/// Invalid input is an expected user error, not a programming error, so it is
/// returned as a message instead of thrown as an exception.
// LEARN: C++23 has std::expected<Query, std::string> for this; in C++20 a small
// LEARN: struct with an optional and a message does the same job.
struct ParseResult {
    std::optional<application::Query> query;  ///< set when the input is valid
    std::string error;                        ///< set when the input is invalid
};

/// @brief Parses and validates one input line.
///
/// Accepted forms (fields separated by spaces or tabs):
/// - 5 numbers: `b e m p u` (s is optional in the specification)
/// - 6 numbers: `b e m p u s` (the written specification)
/// - 7 numbers: `0 b e m p u s` (the examples in the assignment start with an
///   extra 0; it is treated as an optional leading zero, must be 0 and is not
///   used; see "Assumptions and Open Questions" in the README)
///
/// Fields are checked in input order against the rules in Limits.h, so the
/// error names the first invalid field. Search mode (u = 2) requires s.
/// @param line One line as typed by the user; it is not copied.
/// @return A valid Query, or an error message.
// LEARN: std::string_view is a read-only view on existing text: no copy, and it
// LEARN: accepts a std::string, a string literal, or part of a larger buffer.
[[nodiscard]] ParseResult parseInput(std::string_view line);

}  // namespace iban::presentation
