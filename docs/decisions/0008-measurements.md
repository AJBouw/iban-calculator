# 0008. Measurements: Google Benchmark, wall-clock time, local runs

- **Status:** Accepted
- **Date:** decided in the parallel count step (PR #14) and the figures pull request; recorded 2026-11-13

## Context

Every optimisation must be measured against the previous version. Timing
parallel code is easy to get wrong: the work can be optimised away, CPU time of
the calling thread hides the workers, and background load changes the result.

## Options

| Question | Options | Choice and reason |
|---|---|---|
| Tool | `std::chrono` by hand; **Google Benchmark** | Repeats until stable, reports spread, `DoNotOptimize`; from the same team as GoogleTest |
| Time | CPU time (default); **wall-clock time (`UseRealTime`)** | The default CPU time only covers the waiting main thread; wall-clock time is what the user waits |
| Statistic | Mean; **median of 5 repetitions** | One disturbed run does not distort the result; the coefficient of variation shows stability |
| Where | CI runners; **local, Release, other programs closed** | CI runners are shared virtual machines; CI only runs a smoke test and shows indicative numbers |
| Figures | Mermaid only; **Mermaid in Markdown plus a matplotlib figure for the report** | Mermaid renders on GitHub but has no markers, dashed lines or legend; the figure is drawn from committed JSON, so it can be reproduced |

## Decision

`benchmark/CountBenchmark.cpp`, off by default (`IBAN_BUILD_BENCHMARKS`); results,
method and conditions in `docs/measurements.md`; `tools/plot_speedup.py` draws
the figure from `docs/data/*.json`.

## Consequences

- Results are comparable between steps, as long as the conditions are recorded.
- Observed so far: linear speedup up to the 8 performance cores; equal parts on
  unequal cores make 9 to 12 threads slower; background load matters; compare
  times, not only speedups.