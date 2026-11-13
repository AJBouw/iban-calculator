/// @file CountVariantsBenchmark.cpp
/// @brief Measures the ways of combining the thread counts (CountVariants.h)
///        with m = 11 (about 9% of the numbers are valid) and m = 1 (every
///        number is valid, the most additions to the shared counter).
///
/// A separate program from iban_benchmark, so the speedup measurements of the
/// count mode and their CI summary stay unchanged. Run from a Release build:
///   ./build/release/iban_benchmark_variants --benchmark_repetitions=5
///                                           --benchmark_report_aggregates_only=true
// LEARN: The range is smaller than in iban_benchmark: with a lock per number
// LEARN: and m = 1, every one of the numbers takes the lock.

#include <benchmark/benchmark.h>

#include <cstdint>
#include <mutex>

#include "iban/application/CountVariants.h"
#include "iban/application/Query.h"
#include "iban/infrastructure/SpinLock.h"

namespace {

namespace app = iban::application;
using iban::infrastructure::SpinLock;

constexpr std::int64_t kRangeEnd = 10'000'000;

using CountFunction = std::int64_t (*)(const app::Query&);

// state.range(0) is the thread count p, state.range(1) the modulus m.
void countVariant(benchmark::State& state, CountFunction count) {
    app::Query query;
    query.begin = 0;
    query.end = kRangeEnd;
    query.threadCount = static_cast<int>(state.range(0));
    query.modulus = static_cast<int>(state.range(1));

    for (auto _ : state) {
        benchmark::DoNotOptimize(count(query));
    }
}

// Every variant with p = 1, 2, 4, 8 and 16 and with m = 11 and m = 1. Wall-clock
// time, as in iban_benchmark: the CPU time of the waiting main thread says nothing.
// LEARN: BENCHMARK_CAPTURE passes extra arguments (here the function) and adds
// LEARN: a name; ArgsProduct runs every combination of the two argument lists.
// LEARN: benchmark::Benchmark is the registered benchmark; the older name
// LEARN: benchmark::internal::Benchmark is deprecated since v1.10.0.
void variantArguments(benchmark::Benchmark* variant) {
    variant->ArgNames({"p", "m"})
        ->ArgsProduct({{1, 2, 4, 8, 16}, {11, 1}})
        ->UseRealTime()
        ->Unit(benchmark::kMillisecond);
}

}  // namespace

BENCHMARK_CAPTURE(countVariant, result_slots, &app::countWithResultSlots)->Apply(variantArguments);
BENCHMARK_CAPTURE(countVariant, mutex_per_thread, &app::countSharedPerThread<std::mutex>)
    ->Apply(variantArguments);
BENCHMARK_CAPTURE(countVariant, spin_lock_per_thread, &app::countSharedPerThread<SpinLock>)
    ->Apply(variantArguments);
BENCHMARK_CAPTURE(countVariant, atomic_per_hit, &app::countAtomicPerHit)->Apply(variantArguments);
BENCHMARK_CAPTURE(countVariant, mutex_per_hit, &app::countSharedPerHit<std::mutex>)
    ->Apply(variantArguments);
BENCHMARK_CAPTURE(countVariant, spin_lock_per_hit, &app::countSharedPerHit<SpinLock>)
    ->Apply(variantArguments);

BENCHMARK_MAIN();
