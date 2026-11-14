/// @file main.cpp
/// @brief Entry point: asks for the input values, validates them and
///        runs the requested mode. Count and list mode are implemented;
///        search mode follows.
// LEARN: main() only wires the layers together: read text, call the parser,
// LEARN: print the result. All logic lives in iban_core, where it is tested.

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>

#include "iban/Version.h"
#include "iban/application/Mode.h"
#include "iban/application/ParallelCount.h"
#include "iban/application/ParallelList.h"
#include "iban/application/Query.h"
#include "iban/presentation/ConsoleInput.h"
#include "iban/presentation/InputParser.h"

namespace {

using iban::application::Mode;

// Prefix for error messages, so the source is clear in the output of a script.
// LEARN: A fixed name instead of argv[0]: argv[0] can be a full path such as
// LEARN: ./build/debug/iban, which makes the message long and platform
// dependent.
constexpr std::string_view kProgramName = "iban";

// Readable name of a mode for the summary line.
// LEARN: A switch over an enum class without a default: the compiler warns
// LEARN: (-Wswitch) when a new mode is added but not handled here.
std::string_view modeName(Mode mode) {
    switch (mode) {
        case Mode::Count:
            return "count";
        case Mode::List:
            return "list";
        case Mode::Search:
            return "search";
    }
    // Only reached with an invalid enum value; the parser never creates one.
    // LEARN: Some compilers still require a return after a complete switch.
    return "unknown";
}

}  // namespace

int main() {
    // The questions go to standard error, so standard output contains only the
    // results and can be compared or piped (for example in end-to-end tests).
    // LEARN: std::cerr is shown on the screen just like std::cout, but it is a
    // LEARN: separate stream: "iban > out.txt" writes only the results to file.
    std::cerr << "IBAN Calculator " << iban::kVersion << "\n";
    const auto line = iban::presentation::readInputLine(std::cin, std::cerr);
    if (!line) {
        std::cerr << kProgramName << ": error: no input\n";
        return EXIT_FAILURE;
    }

    const auto result = iban::presentation::parseInput(*line);
    if (!result.query) {
        // Exit code 1 lets scripts and tests detect the failure without
        // reading the message.
        std::cerr << kProgramName << ": error: " << result.error << "\n";
        return EXIT_FAILURE;
    }

    const auto& query = *result.query;
    try {
        switch (query.mode) {
            case Mode::Count:
                // Count mode: the output is one number, as in the assignment example.
                std::cout << iban::application::countParallel(query) << "\n";
                return EXIT_SUCCESS;
            case Mode::List:
                // List mode: one line "<sequence number> <number>" per valid
                // number, written by the threads as soon as they find it.
                // LEARN: No std::endl or flush per line: on a terminal the output
                // LEARN: appears line by line anyway, and flushing every line would
                // LEARN: make large lists much slower when the output is piped.
                static_cast<void>(iban::application::listParallel(query, std::cout));
                return EXIT_SUCCESS;
            case Mode::Search:
                break;  // follows; see the temporary output below
        }
    } catch (const std::exception& error) {
        // For example std::system_error when a thread cannot be started.
        // LEARN: Catching here gives a clear message and exit code 1 instead
        // LEARN: of std::terminate with an unhandled exception.
        std::cerr << kProgramName << ": error: " << error.what() << "\n";
        return EXIT_FAILURE;
    }

    // Temporary output for search mode, which is not implemented yet: confirms
    // that the input is valid.
    std::cout << "input OK: " << modeName(query.mode) << " mode, range [" << query.begin << ", "
              << query.end << "), m = " << query.modulus << ", p = " << query.threadCount;
    if (query.target) {
        std::cout << ", s = " << *query.target;
    }
    std::cout << "\n";
    return EXIT_SUCCESS;
}
