/// @file InputParser.cpp
/// @brief Implementation of the input parser in two phases: text to numbers,
///        then numbers to a validated Query. See InputParser.h for the rules.

#include "iban/presentation/InputParser.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <utility>
#include <vector>

#include "iban/application/Limits.h"
#include "iban/application/Mode.h"

namespace iban::presentation {

namespace {

using application::Mode;
using application::Query;
// LEARN: using-declarations inside the anonymous namespace: short names in this
// LEARN: file only, without leaking them to other files.

// Number of fields in the three accepted forms (see InputParser.h).
constexpr std::size_t kFieldsWithoutTarget = 5;    // b e m p u
constexpr std::size_t kFieldsWithLeadingZero = 7;  // 0 b e m p u s

// The assignment examples start with an extra 0, read as an optional leading zero.
constexpr std::int64_t kLeadingZero = 0;

// Mode u as numbers, derived from the enum so the two cannot drift apart.
// LEARN: static_cast converts the enum value to its number (Count = 0, ...).
constexpr std::int64_t kFirstMode = static_cast<std::int64_t>(Mode::Count);
constexpr std::int64_t kLastMode = static_cast<std::int64_t>(Mode::Search);

// Spaces, tabs and line endings (\r from Windows) separate the fields.
constexpr std::string_view kWhitespace = " \t\r\n";

ParseResult failure(std::string message) {
    // LEARN: std::move hands the string over instead of copying it.
    return {std::nullopt, std::move(message)};
}

// Builds "<name> must be between <low> and <high>" from the constants, so a
// message can never disagree with the rule it reports.
std::string rangeMessage(std::string_view name, std::int64_t low, std::int64_t high) {
    return std::string(name) + " must be between " + std::to_string(low) + " and " +
           std::to_string(high);
}

bool inRange(std::int64_t value, std::int64_t low, std::int64_t high) {
    return value >= low && value <= high;
}

// Phase 1: splits the line on whitespace and converts every field to a 64-bit
// integer. Returns nothing and sets `error` when a field is not a complete
// integer or does not fit in 64 bits.
std::optional<std::vector<std::int64_t>> parseNumbers(std::string_view line, std::string& error) {
    std::vector<std::int64_t> numbers;
    std::size_t start = line.find_first_not_of(kWhitespace);
    // LEARN: npos means "not found": the rest of the line is only whitespace.
    while (start != std::string_view::npos) {
        const std::size_t stop = line.find_first_of(kWhitespace, start);
        const std::string_view field = line.substr(start, stop - start);

        // std::from_chars does not throw, does not depend on the locale and
        // reports how many characters it read, so "12abc" is detected.
        // LEARN: Structured binding: from_chars returns {end pointer, error code}
        // LEARN: and auto [end, code] unpacks both into named variables.
        std::int64_t value = 0;
        const char* const fieldEnd = field.data() + field.size();
        const auto [end, code] = std::from_chars(field.data(), fieldEnd, value);
        if (code == std::errc::result_out_of_range) {
            error = "number too large: '" + std::string(field) + "'";
            return std::nullopt;
        }
        if (code != std::errc{} || end != fieldEnd) {
            error = "not an integer: '" + std::string(field) + "'";
            return std::nullopt;
        }
        numbers.push_back(value);
        start = line.find_first_not_of(kWhitespace, stop);
    }
    return numbers;
}

}  // namespace

ParseResult parseInput(std::string_view line) {
    // Phase 1: text to numbers.
    std::string error;
    const auto parsed = parseNumbers(line, error);
    if (!parsed) {
        return failure(error);
    }
    const std::vector<std::int64_t>& numbers = *parsed;

    // Phase 2: numbers to a validated Query, checked in input order so the
    // message names the first invalid field.
    if (numbers.size() < kFieldsWithoutTarget || numbers.size() > kFieldsWithLeadingZero) {
        return failure("expected 5 to 7 numbers (b e m p u [s]), got " +
                       std::to_string(numbers.size()));
    }

    std::size_t next = 0;  // index of the next unread field
    if (numbers.size() == kFieldsWithLeadingZero) {
        // Only a zero is accepted, so a shifted line such as "5 b e m p u s"
        // is reported instead of silently ignoring the 5.
        if (numbers[next] != kLeadingZero) {
            return failure("with 7 numbers the first must be the leading 0");
        }
        ++next;
    }

    // LEARN: numbers[next++] reads the field and then moves to the next one.
    const std::int64_t begin = numbers[next++];
    const std::int64_t end = numbers[next++];
    const std::int64_t modulus = numbers[next++];
    const std::int64_t threadCount = numbers[next++];
    const std::int64_t mode = numbers[next++];
    const bool hasTarget = next < numbers.size();

    // b is checked against the upper bound on its own, so a too large b is
    // reported as a problem with b and not with e.
    if (!inRange(begin, application::kMinBound, application::kMaxBound)) {
        return failure(rangeMessage("b", application::kMinBound, application::kMaxBound));
    }
    if (end <= begin || end > application::kMaxBound) {
        return failure("e must be greater than b and at most " +
                       std::to_string(application::kMaxBound));
    }
    if (!inRange(modulus, application::kMinModulus, application::kMaxModulus)) {
        return failure(rangeMessage("m", application::kMinModulus, application::kMaxModulus));
    }
    if (!inRange(threadCount, application::kMinThreads, application::kMaxThreads)) {
        return failure(rangeMessage("p", application::kMinThreads, application::kMaxThreads));
    }
    if (!inRange(mode, kFirstMode, kLastMode)) {
        return failure("u must be 0 (count), 1 (list) or 2 (search)");
    }
    const auto validMode = static_cast<Mode>(mode);
    if (validMode == Mode::Search && !hasTarget) {
        return failure("search mode (u = 2) requires s");
    }

    // Every value has been checked, so the narrowing casts below are safe.
    Query query;
    query.begin = begin;
    query.end = end;
    query.modulus = static_cast<int>(modulus);
    query.threadCount = static_cast<int>(threadCount);
    query.mode = validMode;
    if (hasTarget) {
        query.target = numbers[next];
    }
    return {query, {}};
}

}  // namespace iban::presentation
