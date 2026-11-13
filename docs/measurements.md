# Measurements

Every optimisation in this project is measured against the previous version, so its effect can be shown rather than assumed. This file records how the measurements are taken and what they show.

## Method

| Aspect | Choice | Reason |
|---|---|---|
| Tool | [Google Benchmark](https://github.com/google/benchmark) v1.10.0, fetched by CMake | The standard library for C++ micro-benchmarks, from the same team as GoogleTest. It repeats each measurement until it is stable and reports the spread. |
| Build | Release, with `-DIBAN_BUILD_BENCHMARKS=ON` | Debug code is not optimised; its timings do not represent the real program. |
| Workload | Count mode on `[0, 10⁸)` with `m = 11`, for 1 to 12, 16 and 32 threads | Large enough that starting threads is negligible next to the work. Every count up to 12 shows where all cores are in use; 16 and 32 show more threads than cores. |
| Time | Wall-clock time (`UseRealTime`) | The default CPU time only covers the calling thread, which just waits for the workers. Wall-clock time is what the user waits. |
| Repeats | 5 repetitions, median reported | One slow run (another program using the CPU) does not distort the result. The coefficient of variation (cv) shows how stable the runs were. |
| Correctness | Checked by the unit tests (`ParallelCount.MatchesSequentialForEveryThreadCount`) | A benchmark measures time only; correctness is the job of the tests. |
| Conditions | Laptop on mains power, other programs closed | Battery saving and background load change the clock speed. |

**Speedup** = median time with 1 thread / median time with p threads.
**Efficiency** = speedup / p; 100% means every thread does useful work all the time.

### Commands

```bash
cmake --preset release -DIBAN_BUILD_BENCHMARKS=ON
cmake --build --preset release
./build/release/iban_benchmark --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
```

Use the `real_time_median` rows; the `CPU` column is the time of the waiting main thread and is not used.

## Machine

| | |
|---|---|
| Computer | Apple M1 Pro |
| Cores | 10: 8 performance cores and 2 efficiency cores (`sysctl hw.perflevel0.physicalcpu hw.perflevel1.physicalcpu`) |
| Compiler | AppleClang 21.0.0 |
| Date | 2026-11-13 |

The benchmark also prints the number of CPUs and the cache sizes at the top of its output.

## Step 4: parallel count, one result slot per thread

### First measurement (2026-11-13, thread counts 1, 2, 4, …, 32)

Measured with background load (load average about 12), so the numbers are somewhat pessimistic.

| Threads | Median (ms) | cv | Speedup | Efficiency |
|---:|---:|---:|---:|---:|
| 1 | 579 | 1.9% | 1.00 | 100% |
| 2 | 294 | 0.4% | 1.97 | 98% |
| 4 | 153 | 2.5% | 3.78 | 95% |
| 8 | 88.2 | 5.6% | 6.56 | 82% |
| 16 | 90.8 | 7.0% | 6.38 | 40% |
| 32 | 90.4 | 6.2% | 6.40 | 20% |

The chart compares the measured speedup with the ideal speedup (equal to the number of threads, at most 10 cores):

```mermaid
---
config:
  themeVariables:
    xyChart:
      plotColorPalette: "#2563eb, #f59e0b"
---
xychart-beta
    title "Speedup, count mode, [0, 10^8), m = 11"
    x-axis "threads" [1, 2, 4, 8, 16, 32]
    y-axis "speedup" 0 --> 10
    line [1.00, 1.97, 3.78, 6.56, 6.38, 6.40]
    line [1, 2, 4, 8, 10, 10]
```

**Legend:** blue line = measured speedup; orange line = ideal speedup.

### Second measurement (2026-11-13, thread counts 1 to 12, 16 and 32)

Measured with less background load (load average about 6) and every thread count from 1 to 12, so the effect of the two kinds of cores is visible.

| Threads | Median (ms) | cv | Speedup | Efficiency |
|---:|---:|---:|---:|---:|
| 1 | 608 | 5.0% | 1.00 | 100% |
| 2 | 298 | 2.0% | 2.04 | 102% |
| 3 | 202 | 3.3% | 3.01 | 100% |
| 4 | 150 | 0.6% | 4.05 | 101% |
| 5 | 121 | 2.8% | 5.02 | 100% |
| 6 | 101 | 1.4% | 6.02 | 100% |
| 7 | 88.0 | 1.7% | 6.91 | 99% |
| 8 | **78.2** | 1.8% | **7.77** | 97% |
| 9 | 85.7 | 2.7% | 7.09 | 79% |
| 10 | 84.0 | 1.8% | 7.24 | 72% |
| 11 | 87.3 | 4.9% | 6.96 | 63% |
| 12 | 89.5 | 4.0% | 6.79 | 57% |
| 16 | 81.6 | 3.7% | 7.45 | 47% |
| 32 | 82.8 | 5.2% | 7.34 | 23% |

The chart compares the measured speedup with the ideal speedup (equal to the number of threads, at most 10 cores):

```mermaid
---
config:
  themeVariables:
    xyChart:
      plotColorPalette: "#2563eb, #f59e0b"
---
xychart-beta
    title "Speedup, count mode, M1 Pro (8 P + 2 E cores)"
    x-axis "threads" [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16, 32]
    y-axis "speedup" 0 --> 10
    line [1.00, 2.04, 3.01, 4.05, 5.02, 6.02, 6.91, 7.77, 7.09, 7.24, 6.96, 6.79, 7.45, 7.34]
    line [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10]
```

**Legend:** blue line = measured speedup; orange line = ideal speedup.

### Third measurement (2026-11-13, other programs closed)

Measured with all other programs closed (load average about 4.6). The raw results are in [`data/step4-count.json`](data/step4-count.json); the figure below is drawn from that file.

| Threads | Median (ms) | cv | Speedup | Efficiency |
|---:|---:|---:|---:|---:|
| 1 | 578 | 0.3% | 1.00 | 100% |
| 2 | 292 | 2.4% | 1.98 | 99% |
| 3 | 199 | 0.2% | 2.90 | 97% |
| 4 | 150 | 0.3% | 3.85 | 96% |
| 5 | 120 | 0.1% | 4.82 | 96% |
| 6 | 101 | 1.6% | 5.72 | 95% |
| 7 | 86.7 | 0.5% | 6.67 | 95% |
| 8 | 77.0 | 1.5% | 7.51 | 94% |
| 9 | 82.2 | 2.0% | 7.03 | 78% |
| 10 | 83.0 | 5.3% | 6.96 | 70% |
| 11 | 84.7 | 4.1% | 6.82 | 62% |
| 12 | 80.6 | 3.8% | 7.17 | 60% |
| 16 | 80.8 | 1.6% | 7.15 | 45% |
| 32 | **74.8** | 1.4% | **7.73** | 24% |

```mermaid
---
config:
  themeVariables:
    xyChart:
      plotColorPalette: "#2563eb, #f59e0b"
---
xychart-beta
    title "Speedup, count mode, M1 Pro, other programs closed"
    x-axis "threads" [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16, 32]
    y-axis "speedup" 0 --> 10
    line [1.00, 1.98, 2.90, 3.85, 4.82, 5.72, 6.67, 7.51, 7.03, 6.96, 6.82, 7.17, 7.15, 7.73]
    line [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10]
```

**Legend:** blue line = measured speedup; orange line = ideal speedup.

### Observations

- **Linear speedup up to 8 threads.** Up to the 8 performance cores the efficiency stays near 100%: the threads share nothing while they run, so they do not wait for each other. Values just above 100% are within the measurement spread (cv up to 5%).
- **Of 1 to 12 threads, 8 is the fastest, not 10.** With 9 or 10 threads the program becomes *slower*, although there are 10 cores. This holds in all three measurements.
- **Cause: equal parts on unequal cores.** The range is split into equal parts, as the assignment requires. With 9 or more threads, some parts run on the 2 efficiency cores, which are considerably slower. The count is finished only when the slowest thread is finished, so these parts determine the total time.
- **16 and 32 threads are faster than 9 to 12.** With more threads than cores the parts are smaller, and the operating system moves threads between cores; a thread that lands on an efficiency core holds up only a small piece of work. In the third measurement 32 threads were even slightly faster than 8 (74.8 against 77.0 ms, 3%). More, smaller parts thus act as a simple form of dynamic distribution, but the gain is small because there are no more fast cores.
- **Conclusion:** a fair static split is optimal for identical cores. On a processor with performance and efficiency cores, the best choice here is the number of performance cores, or many more threads than cores. Dynamic work distribution (many small tasks taken by whichever thread is free) balances unequal cores on purpose; this is investigated with the thread pool.
- **Effect of background load.** At 8 threads the time was 88.2 ms with a load average of about 12 (first measurement), 78.2 ms at about 6 (second) and 77.0 ms at about 4.6 (third). Background load clearly slows the program down, so the conditions are recorded with every measurement. Closing programs also made the runs more stable: in the third measurement the spread up to 8 threads is at most 2.4%.
- **Compare times, not only speedups.** The speedup at 8 threads is lower in the third measurement (7.5×) than in the second (7.8×), although 8 threads were just as fast (77.0 against 78.2 ms). The difference comes from the run with 1 thread (578 against 608 ms), which is the base of every speedup: a slow base makes every speedup look better.

## Step 5: shared counter

The assignment asks count mode to keep the result in a shared counter. This step measures what that costs, and which way of using the counter is cheapest. The reasons for the final choice are in [decision 0010](decisions/0010-shared-counter-and-lock.md).

### What is compared

Every variant splits the range the same way and gives the same count (checked by `CountVariantsTest`); they differ only in how the threads combine their results.

| Variant | Shared while the threads run | Synchronisations |
|---|---|---|
| `result_slots` | nothing: one slot per thread, added after join (step 4, the baseline) | none |
| `mutex_per_thread` | a counter with `std::mutex`, one locked add per thread (**the program**) | p |
| `spin_lock_per_thread` | the same with `SpinLock` | p |
| `atomic_per_hit` | a `std::atomic` counter, `fetch_add` per valid number | one per valid number |
| `mutex_per_hit` | a counter with `std::mutex`, locked per valid number | one per valid number |
| `spin_lock_per_hit` | the same with `SpinLock` | one per valid number |

Each variant runs with p = 1, 2, 4, 8 and 16 and with two moduli: **m = 11** (about 9% of the numbers are valid) and **m = 1** (every number is valid, so the variants "per hit" synchronise for every number: the worst case). The range is `[0, 10⁷)`, smaller than in step 4, because a lock per number with m = 1 makes the slow variants take seconds.

### Commands

```bash
cmake --preset release -DIBAN_BUILD_BENCHMARKS=ON
cmake --build --preset release
./build/release/iban_benchmark_variants --benchmark_repetitions=5 --benchmark_report_aggregates_only=true \
    --benchmark_out=docs/data/step5-variants.json --benchmark_out_format=json
```

As in step 4: other programs closed, load average below about 2 (`uptime`), median of 5 repetitions.

### Expected outcome

- `mutex_per_thread` and `spin_lock_per_thread` as fast as `result_slots`: p synchronisations disappear next to millions of m-checks.
- The variants per hit slower with more threads, and much slower with m = 1, because all threads wait for the same lock or the same cache line (contention).
- `SpinLock` faster than `std::mutex` per hit while p is at most the number of cores, and worse beyond that, when a waiting thread spins on a core the lock holder needs.

### Results

To be added after the measurement on the M1 Pro.

## Figures for the report

The Mermaid charts above are rendered by GitHub, but Mermaid cannot draw markers, dashed lines or a real legend. For the report, `tools/plot_speedup.py` draws the conventional figure from the raw benchmark output: measured speedup with markers, the ideal speedup as a dashed line, the number of performance cores, and the efficiency below it.

```bash
# Save the raw results next to the console output:
./build/release/iban_benchmark --benchmark_repetitions=5 --benchmark_report_aggregates_only=true \
    --benchmark_out=docs/data/step4-count.json --benchmark_out_format=json

# Draw the figure (once: python3 -m venv .venv && .venv/bin/pip install -r tools/requirements.txt):
.venv/bin/python tools/plot_speedup.py docs/data/step4-count.json \
    --output docs/figures/step4-speedup.svg --performance-cores 8 \
    --title "Speedup of the count mode, M1 Pro (8 P + 2 E cores)"
```

![Speedup and efficiency of the count mode, third measurement](figures/step4-speedup.svg)

The JSON file is committed with the figure, so the figure can always be redrawn from the measured data.

## Runs in CI

Every pull request also builds the benchmark and runs each thread count once (`benchmark (smoke run)` in the CI workflow). This only checks that the benchmark still builds and runs. The results are shown as a table on the Summary page of the run and kept as the artifact `benchmark-ubuntu` (JSON), but they are **indicative only**: CI runners are shared virtual machines with few cores and varying hardware, so their timings are not used for the conclusions in this file.

## References

The benchmark follows the official Google Benchmark documentation. After a build with `-DIBAN_BUILD_BENCHMARKS=ON`, the documentation of the exact version used is also available locally in `build/release/_deps/googlebenchmark-src/docs/`.

| Topic | Source | Used for |
|---|---|---|
| Basic structure, CMake integration | [Google Benchmark README](https://github.com/google/benchmark#readme), sections *Usage* and *Usage with CMake* | `benchmark::State`, the measuring loop, `BENCHMARK_MAIN`, `FetchContent` |
| Thread counts as arguments | [User Guide](https://github.com/google/benchmark/blob/main/docs/user_guide.md), *Passing Arguments* | `Range(1, 32)` with `RangeMultiplier(2)` |
| Wall-clock time for code that starts threads | User Guide, *CPU Timers* | `UseRealTime()`: the default CPU timer only measures the main thread |
| Keeping the work from being optimised away | User Guide, *Preventing Optimization* | `benchmark::DoNotOptimize` |
| Repetitions, median and spread | User Guide, *Statistics: Reporting the Mean, Median and Standard Deviation* | `--benchmark_repetitions`, `--benchmark_report_aggregates_only` |
| Stable measurements | [Reducing Variance](https://github.com/google/benchmark/blob/main/docs/reducing_variance.md) | The *Conditions* row of the method |
| Why the speedup levels off | [Amdahl's law](https://en.wikipedia.org/wiki/Amdahl%27s_law) | Interpreting the observations |
| Measuring C++ code in general | Chandler Carruth, *Tuning C++: Benchmarks, and CPUs, and Compilers! Oh My!*, CppCon 2015 | Background on benchmarks and `DoNotOptimize` |

