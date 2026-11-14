/// @file ListVariantsBenchmark.cpp
/// @brief Measures the ways of writing the list (ListVariants.h) with m = 11
///        and m = 1. The lines go to a stream that discards them, so the
///        measurement shows the cost of the synchronisation and formatting,
///        not of the terminal or the disk.
///
/// Part of iban_benchmark_variants:
///   ./build/release/iban_benchmark_variants --benchmark_filter=listVariant
// LEARN: A std::ostream needs a stream buffer; one whose overflow() accepts and
// LEARN: drops every character turns the stream into a "null" output.

#include <benchmark/benchmark.h>

#include <cstdint>
#include <mutex>
#include <ostream>
#include <streambuf>

#include "iban/application/ListVariants.h"
#include "iban/application/ParallelList.h"
#include "iban/application/Query.h"
#include "iban/infrastructure/SpinLock.h"

namespace {

namespace app = iban::application;
using iban::infrastructure::SpinLock;

// With m = 1 every number is a line, so the range is smaller than for counting.
constexpr std::int64_t kRangeEnd = 1'000'000;

class NullBuffer : public std::streambuf {
protected:
    int_type overflow(int_type character) override { return character; }
    std::streamsize xsputn(const char_type* /*text*/, std::streamsize count) override {
        return count;
    }
};

using ListFunction = std::int64_t (*)(const app::Query&, std::ostream&);

// state.range(0) is the thread count p, state.range(1) the modulus m.
void listVariant(benchmark::State& state, ListFunction list) {
    app::Query query;
    query.begin = 0;
    query.end = kRangeEnd;
    query.threadCount = static_cast<int>(state.range(0));
    query.modulus = static_cast<int>(state.range(1));
    query.mode = app::Mode::List;

    NullBuffer buffer;
    std::ostream out(&buffer);
    for (auto _ : state) {
        benchmark::DoNotOptimize(list(query, out));
    }
}

std::int64_t writerThread(const app::Query& query, std::ostream& out) {
    return app::listWithWriterThread(query, out);
}

void listArguments(benchmark::Benchmark* variant) {
    variant->ArgNames({"p", "m"})
        ->ArgsProduct({{1, 2, 4, 8, 16}, {11, 1}})
        ->UseRealTime()
        ->Unit(benchmark::kMillisecond);
}

}  // namespace

BENCHMARK_CAPTURE(listVariant, mutex, &app::listWithLock<std::mutex>)->Apply(listArguments);
BENCHMARK_CAPTURE(listVariant, spin_lock, &app::listWithLock<SpinLock>)->Apply(listArguments);
BENCHMARK_CAPTURE(listVariant, writer_thread, &writerThread)->Apply(listArguments);
