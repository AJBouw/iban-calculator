/// @file main.cpp
/// @brief Entry point: asks for the input values, validates them and
///        runs the requested mode: count, list, or search.
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
#include "iban/application/ParallelSearch.h"
#include "iban/application/Query.h"
#include "iban/presentation/ConsoleInput.h"
#include "iban/presentation/InputParser.h"

namespace {

using iban::application::Mode;

// Prefix for error messages, so the source is clear in the output of a script.
// LEARN: A fixed name instead of argv[0]: argv[0] can be a full path such as
// LEARN: ./build/debug/iban, which makes the message long and platform
// LEARN: dependent.
constexpr std::string_view kProgramName = "iban";

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
                // Search mode: s when it is found, otherwise -1. The other
                // threads have stopped when searchParallel returns.
                std::cout << iban::application::searchParallel(query) << "\n";
                return EXIT_SUCCESS;
        }
    } catch (const std::exception& error) {
        // For example std::system_error when a thread cannot be started.
        // LEARN: Catching here gives a clear message and exit code 1 instead
        // LEARN: of std::terminate with an unhandled exception.
        std::cerr << kProgramName << ": error: " << error.what() << "\n";
        return EXIT_FAILURE;
    }

    // Every case of the switch returns; this is only reached with an invalid
    // mode, which the parser never creates.
    return EXIT_FAILURE;
}
