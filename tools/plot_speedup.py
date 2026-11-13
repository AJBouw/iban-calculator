#!/usr/bin/env python3
"""Draws the speedup and efficiency of a Google Benchmark run as a figure.

Input is the JSON file written by the benchmark with
    --benchmark_out=<file> --benchmark_out_format=json
With repetitions, the median of every thread count is used; without them, the
single run. Speedup = time with 1 thread / time with p threads; efficiency =
speedup / p.

Usage:
    python3 tools/plot_speedup.py docs/data/step4-count.json \
        --output docs/figures/step4-speedup.svg --performance-cores 8
"""

import argparse
import json
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # draw to a file, no window needed
import matplotlib.pyplot as plt  # noqa: E402  (must follow matplotlib.use)

# Factor to convert each Google Benchmark time unit to milliseconds.
TO_MILLISECONDS = {"ns": 1e-6, "us": 1e-3, "ms": 1.0, "s": 1e3}


def read_times(path):
    """Returns ({threads: milliseconds}, number of CPUs) from a benchmark JSON file."""
    data = json.loads(Path(path).read_text(encoding="utf-8"))
    runs = data["benchmarks"]

    # With repetitions, use the median aggregate; otherwise every plain run.
    medians = [run for run in runs if run.get("aggregate_name") == "median"]
    selected = medians or [run for run in runs if run.get("run_type", "iteration") == "iteration"]

    times = {}
    for run in selected:
        # Names look like "countParallelBenchmark/8/real_time_median".
        threads = int(run["name"].split("/")[1])
        times[threads] = run["real_time"] * TO_MILLISECONDS[run["time_unit"]]
    if 1 not in times:
        sys.exit("error: the file has no result for 1 thread, so no speedup can be computed")
    return dict(sorted(times.items())), data["context"].get("num_cpus")


def plot(times, cpus, title, performance_cores, output):
    threads = list(times)
    speedup = [times[1] / times[p] for p in threads]
    efficiency = [s / p * 100 for s, p in zip(speedup, threads)]
    # Ideal: p threads are p times faster, but never more than the number of cores.
    ideal = [min(p, cpus) if cpus else p for p in threads]

    figure, (top, bottom) = plt.subplots(
        2, 1, figsize=(8, 8), sharex=True, gridspec_kw={"height_ratios": [2, 1]})

    top.plot(threads, ideal, linestyle="--", color="grey", label="ideal speedup")
    top.plot(threads, speedup, marker="o", color="tab:blue", label="measured speedup")
    top.set_ylabel("speedup (× faster than 1 thread)")
    top.set_title(title)

    bottom.plot(threads, efficiency, marker="o", color="tab:orange", label="efficiency")
    bottom.axhline(100, linestyle="--", color="grey", linewidth=1)
    bottom.set_ylabel("efficiency (%)")
    bottom.set_xlabel("threads")
    bottom.set_ylim(0, 110)

    if performance_cores:
        for axis in (top, bottom):
            axis.axvline(performance_cores, linestyle=":", color="tab:green",
                         label=f"{performance_cores} performance cores" if axis is top else None)

    # Linear axes, so the ideal speedup is a straight diagonal; ticks only at
    # the measured thread counts.
    for axis in (top, bottom):
        axis.set_xticks(threads, [str(p) for p in threads])
        axis.grid(True, alpha=0.3)
    top.legend()

    figure.tight_layout()
    # Create the folder (e.g. docs/figures) if it does not exist yet.
    Path(output).parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(output)
    print(f"written: {output}")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("input", help="benchmark JSON file")
    parser.add_argument("--output", default="speedup.svg", help="figure file (.svg or .png)")
    parser.add_argument("--title", default="Speedup of the count mode", help="figure title")
    parser.add_argument("--performance-cores", type=int, default=0,
                        help="draw a marker at this thread count (e.g. 8 on an M1 Pro)")
    arguments = parser.parse_args()

    times, cpus = read_times(arguments.input)
    plot(times, cpus, arguments.title, arguments.performance_cores, arguments.output)


if __name__ == "__main__":
    main()
