/// @file ConsoleInput.cpp
/// @brief Implementation of the console input. See ConsoleInput.h.

#include "iban/presentation/ConsoleInput.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string_view>

#include "iban/application/Limits.h"

namespace iban::presentation {

namespace {

namespace app = iban::application;

// One question to the user: the letter from the assignment, what it means and
// which values are allowed.
struct Field {
    char letter;
    std::string_view description;
    std::string allowed;
    bool optional;
};

constexpr std::string_view kWhitespace = " \t\r\n";
constexpr std::size_t kDescriptionWidth = 26;

// "low to high", built from the limits so the text matches the validation.
std::string range(std::int64_t low, std::int64_t high) {
    return std::to_string(low) + " to " + std::to_string(high);
}

// The questions in input order. The ranges come from Limits.h.
// LEARN: std::array has a fixed size known at compile time, unlike std::vector.
std::array<Field, 6> fields() {
    return {{
        {'b', "lower bound, inclusive", range(app::kMinBound, app::kMaxBound), false},
        {'e', "upper bound, exclusive", "greater than b, at most " + std::to_string(app::kMaxBound),
         false},
        {'m', "modulus of the m-check", range(app::kMinModulus, app::kMaxModulus), false},
        {'p', "number of threads", range(app::kMinThreads, app::kMaxThreads), false},
        {'u', "mode", "0 = count, 1 = list, 2 = search", false},
        {'s', "number to search for", "required when u = 2, press Enter to skip", true},
    }};
}

void ask(std::ostream& prompt, const Field& field) {
    // LEARN: Padding with spaces instead of std::setw: setw needs std::left,
    // LEARN: which stays set on the stream and could affect later output.
    const std::size_t padding = field.description.size() < kDescriptionWidth
                                    ? kDescriptionWidth - field.description.size()
                                    : 1;
    prompt << "  " << field.letter << "  " << field.description << std::string(padding, ' ') << '('
           << field.allowed << "): ";
}

bool isBlank(std::string_view text) {
    return text.find_first_not_of(kWhitespace) == std::string_view::npos;
}

// True when the text contains more than one whitespace-separated value.
bool hasSeveralValues(std::string_view text) {
    const std::size_t first = text.find_first_not_of(kWhitespace);
    if (first == std::string_view::npos) {
        return false;
    }
    const std::size_t gap = text.find_first_of(kWhitespace, first);
    return gap != std::string_view::npos &&
           text.find_first_not_of(kWhitespace, gap) != std::string_view::npos;
}

}  // namespace

std::optional<std::string> readInputLine(std::istream& in, std::ostream& prompt) {
    // The assignment examples start with an extra 0 that is not a field; typing
    // it as b would shift every value by one question.
    prompt << "Enter each value and press Enter, or paste all values on one line.\n"
           << "The assignment examples start with an extra 0: use it only on one line.\n";

    std::string line;
    bool first = true;
    for (const Field& field : fields()) {
        std::string answer;
        // A required value is asked again after an empty answer.
        do {
            ask(prompt, field);
            if (!std::getline(in, answer)) {
                // End of input: nothing more to read. The values so far are
                // still returned, so the parser can report what is missing.
                return line.empty() ? std::nullopt : std::optional<std::string>{line};
            }
        } while (isBlank(answer) && !field.optional);

        // A complete line was pasted or piped at the first question.
        if (first && hasSeveralValues(answer)) {
            return answer;
        }
        first = false;

        if (!isBlank(answer)) {
            line += (line.empty() ? "" : " ") + answer;
        }
    }
    return line;
}

}  // namespace iban::presentation
