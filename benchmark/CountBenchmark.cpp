/// @file CountBenchmark.cpp
/// @brief Measures the count mode with 1 to 12, 16 and 32 threads using Google
///        Benchmark. Speedup and efficiency are derived from the reported
///        times in docs/measurements.md.
///
/// Run from a Release build:
///   ./build/release/iban_benchmark --benchmark_repetitions=5
///                                  --benchmark_report_aggregates_only=true
// LEARN: A separate program, not a unit test: tests check correctness and must
// LEARN: be fast; a benchmark measures time and may take a while.

#include <benchmark/benchmark.h>

#include <cstdint>

#include "iban/application/ParallelCount.h"
#include "iban/application/Query.h"

namespace {

namespace app = iban::application;

// Large enough that starting threads is negligible next to the work itself.
constexpr std::int64_t kRangeEnd = 100'000'000;
constexpr int kModulus = 11;

// One benchmark run per thread count; state.range(0) is the thread count.
void countParallelBenchmark(benchmark::State& state) {
    app::Query query;
    query.begin = 0;
    query.end = kRangeEnd;
    query.modulus = kModulus;
    query.threadCount = static_cast<int>(state.range(0));

    // LEARN: The library repeats this loop until the time is stable enough.
    // LEARN: The loop variable is not used; the name _ is the usual convention.
    for (auto _ : state) {
        // DoNotOptimize keeps the compiler from removing a call whose result
        // would otherwise be unused.
        benchmark::DoNotOptimize(app::countParallel(query));
    }
}

}  // namespace

// Thread counts 1 to 12 one by one, so the point where all cores are in use is
// visible, then 16 and 32 to show using more threads than cores.
// UseRealTime measures wall-clock time. The default CPU time only covers the
// calling thread, which just waits for the worker threads, so it would show
// almost nothing.
// LEARN: DenseRange(1, 12) gives every value 1, 2, ..., 12; Arg adds one value.
// LEARN: ->MeasureProcessCPUTime() would add up the CPU time of all threads;
// LEARN: that shows the total work, not how long the user has to wait.
BENCHMARK(countParallelBenchmark)
    ->DenseRange(1, 12)
    ->Arg(16)
    ->Arg(32)
    ->UseRealTime()
    ->Unit(benchmark::kMillisecond);

// LEARN: BENCHMARK_MAIN provides main(), which parses the --benchmark_* options.
BENCHMARK_MAIN();
