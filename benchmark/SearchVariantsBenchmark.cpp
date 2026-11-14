/// @file SearchVariantsBenchmark.cpp
/// @brief Measures what the stop flag saves and costs in search mode
///        (SearchVariants.h): never looking at the flag, looking before every
///        number, and looking before every 1024th number, for an s early in
///        the first part, an s at the end of the last part, and an s that is
///        not in the range.
///
/// Part of iban_benchmark_variants:
///   ./build/release/iban_benchmark_variants --benchmark_filter=searchVariant
// LEARN: RegisterBenchmark builds the names at run time, so every combination of
// LEARN: variant and position gets a readable name such as searchVariant/early/every_number.

#include <benchmark/benchmark.h>

#include <cstdint>
#include <string>

#include "iban/application/Query.h"
#include "iban/application/SearchVariants.h"
#include "iban/domain/MCheck.h"

namespace {

namespace app = iban::application;
using iban::domain::passesMCheck;

// The same range as the count benchmark, so the times can be compared.
constexpr std::int64_t kRangeEnd = 10'000'000;
constexpr int kModulus = 11;

using SearchFunction = std::int64_t (*)(const app::Query&);

// The first valid number from 1000 on: early in the part of the first thread.
std::int64_t earlyTarget() {
    std::int64_t number = 1'000;
    while (!passesMCheck(number, kModulus)) {
        ++number;
    }
    return number;
}

// The last valid number in the range: at the end of the part of the last thread.
std::int64_t lateTarget() {
    std::int64_t number = kRangeEnd - 1;
    while (!passesMCheck(number, kModulus)) {
        --number;
    }
    return number;
}

// state.range(0) is the thread count p.
void searchVariant(benchmark::State& state, SearchFunction search, std::int64_t target) {
    app::Query query;
    query.begin = 0;
    query.end = kRangeEnd;
    query.modulus = kModulus;
    query.threadCount = static_cast<int>(state.range(0));
    query.mode = app::Mode::Search;
    query.target = target;
    for (auto _ : state) {
        benchmark::DoNotOptimize(search(query));
    }
}

struct Variant {
    const char* name;
    SearchFunction search;
};

struct Position {
    const char* name;
    std::int64_t target;
};

// Registers every combination before main() runs.
const bool kRegistered = [] {
    const Variant variants[] = {
        {"never_stop", &app::searchWithStopFlag<app::kNeverStop>},
        {"every_number", &app::searchWithStopFlag<1>},
        {"every_1024", &app::searchWithStopFlag<1024>},
    };
    const Position positions[] = {
        {"early", earlyTarget()},
        {"late", lateTarget()},
        {"absent", kRangeEnd},  // e is exclusive, so s = e is never found
    };
    for (const Position& position : positions) {
        for (const Variant& variant : variants) {
            const std::string name =
                std::string("searchVariant/") + position.name + "/" + variant.name;
            benchmark::RegisterBenchmark(name, &searchVariant, variant.search, position.target)
                ->ArgName("p")
                ->RangeMultiplier(2)
                ->Range(1, 16)
                ->UseRealTime()
                ->Unit(benchmark::kMillisecond);
        }
    }
    return true;
}();

}  // namespace
