/// @file ListVariantsTest.cpp
/// @brief Every way of writing the list must write each valid number exactly
///        once, as "<sequence number> <number>", with the sequence numbers
///        1, 2, 3, ... in the order of the lines.
// LEARN: The order of the account numbers differs per run, so the test sorts
// LEARN: them before comparing; the sequence numbers must not be sorted.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "iban/application/Limits.h"
#include "iban/application/ListVariants.h"
#include "iban/application/ParallelList.h"
#include "iban/application/Query.h"
#include "iban/domain/ForEachValid.h"
#include "iban/domain/Range.h"
#include "iban/infrastructure/SpinLock.h"

namespace app = iban::application;
using iban::domain::forEachValid;
using iban::domain::Range;
using iban::infrastructure::SpinLock;

namespace {

using ListFunction = std::function<std::int64_t(const app::Query&, std::ostream&)>;

const std::vector<std::pair<std::string, ListFunction>>& variants() {
    static const std::vector<std::pair<std::string, ListFunction>> list = {
        {"program", app::listParallel},
        {"mutex", app::listWithLock<std::mutex>},
        {"spin lock", app::listWithLock<SpinLock>},
        {"writer thread", [](const app::Query& query,
                             std::ostream& out) { return app::listWithWriterThread(query, out); }},
        {"writer thread, buffer of 1",
         [](const app::Query& query, std::ostream& out) {
             return app::listWithWriterThread(query, out, 1);
         }},
    };
    return list;
}

app::Query makeQuery(Range range, int modulus, int threads) {
    app::Query query;
    query.begin = range.begin;
    query.end = range.end;
    query.modulus = modulus;
    query.threadCount = threads;
    query.mode = app::Mode::List;
    return query;
}

// Checks the text of a list and returns the account numbers, sorted. Every
// line must be "<sequence number> <number>" with sequence numbers 1, 2, 3, ...
std::vector<std::int64_t> parseList(const std::string& text, const std::string& context) {
    std::istringstream lines(text);
    std::vector<std::int64_t> numbers;
    std::string line;
    std::int64_t expectedSequence = 1;
    while (std::getline(lines, line)) {
        std::istringstream fields(line);
        std::int64_t sequence = 0;
        std::int64_t number = 0;
        std::string extra;
        EXPECT_TRUE(fields >> sequence >> number) << context << ": bad line '" << line << "'";
        EXPECT_FALSE(fields >> extra) << context << ": extra text in '" << line << "'";
        EXPECT_EQ(sequence, expectedSequence) << context << ": line '" << line << "'";
        ++expectedSequence;
        numbers.push_back(number);
    }
    std::sort(numbers.begin(), numbers.end());
    return numbers;
}

std::vector<std::int64_t> expectedNumbers(Range range, int modulus) {
    std::vector<std::int64_t> numbers;
    forEachValid(range, modulus, [&numbers](std::int64_t number) { numbers.push_back(number); });
    return numbers;
}

}  // namespace

TEST(ListVariants, WriteTheAssignmentExample) {
    for (const auto& [name, list] : variants()) {
        std::ostringstream out;
        const std::int64_t written = list(makeQuery(Range{274856170, 274856190}, 11, 4), out);
        EXPECT_EQ(written, 2) << name;
        EXPECT_EQ(parseList(out.str(), name), (std::vector<std::int64_t>{274856174, 274856182}))
            << name;
    }
}

TEST(ListVariants, WriteEveryValidNumberOnceForEveryThreadCount) {
    const Range range{2'000'000, 2'020'000};
    // m = 1: every number is valid, so every number is a line and the threads
    // compete for the output as much as possible.
    for (const int modulus : {1, 11, 97}) {
        const std::vector<std::int64_t> expected = expectedNumbers(range, modulus);
        for (const auto& [name, list] : variants()) {
            for (const int threads : {1, 2, 3, 8, 16}) {
                const std::string context =
                    name + ", m = " + std::to_string(modulus) + ", p = " + std::to_string(threads);
                std::ostringstream out;
                const std::int64_t written = list(makeQuery(range, modulus, threads), out);
                EXPECT_EQ(written, static_cast<std::int64_t>(expected.size())) << context;
                EXPECT_EQ(parseList(out.str(), context), expected) << context;
            }
        }
    }
}

TEST(ListVariants, WriteNothingWhenNoNumberIsValid) {
    // 274856183 does not pass the 11-check.
    for (const auto& [name, list] : variants()) {
        std::ostringstream out;
        EXPECT_EQ(list(makeQuery(Range{274856183, 274856184}, 11, 4), out), 0) << name;
        EXPECT_EQ(out.str(), "") << name;
    }
}

TEST(ListVariants, MoreThreadsThanNumbers) {
    for (const auto& [name, list] : variants()) {
        std::ostringstream out;
        static_cast<void>(list(makeQuery(Range{0, 3}, 1, app::kMaxThreads), out));
        EXPECT_EQ(parseList(out.str(), name), (std::vector<std::int64_t>{0, 1, 2})) << name;
    }
}
