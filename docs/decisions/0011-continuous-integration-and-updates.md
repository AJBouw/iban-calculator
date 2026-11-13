# 0011. Continuous integration and dependency updates

- **Status:** Accepted
- **Date:** decided in the project setup and extended in later steps (PR #9, #14, the end-to-end and Dependabot pull requests); recorded 2026-11-13

## Context

Every pull request must show that the program builds and works on the
platforms an examiner may use, that the code is formatted, and that the
benchmark and the end-to-end tests still run. Pinned dependency versions keep
builds reproducible, but they also age. The repository is private on a free
plan, so branch protection is not available.

## Options

| Question | Options | Choice and reason |
|---|---|---|
| CI service | Local checks only; **GitHub Actions** | Runs on every push and pull request next to the code; free for this use |
| Platforms | Linux only; **Linux, macOS and Windows** | The program is portable C++20; MSVC, GCC and AppleClang each catch different mistakes. `fail-fast: false`, so one failing platform does not hide the others |
| Formatting | Review by eye; **`format-check` job with the pinned clang-format** | Independent of IDE settings (see [0003](0003-code-style-and-workflow.md)) |
| Benchmark in CI | None; full measurement; **smoke run** | Every thread count once (`--benchmark_min_time=1x`): proves the benchmark builds and runs. Shared runners vary too much for conclusions, so the numbers are shown as indicative only (job summary with table and chart, JSON artifact); measurements are taken locally (see [0008](0008-measurements.md)) |
| End-to-end tests in CI | Every platform; **Linux only, with the HTML report as artifact** | The scenarios test behaviour, which the build jobs already check on all platforms; the report is kept also when a scenario fails |
| Superseded runs | Let them finish; **cancel on a newer push** (`concurrency`) | Saves runner time and shows the result of the latest commit |
| Dependency updates | By hand; **Dependabot, daily, against `develop`** | Proposes updates for GitHub Actions and Maven with Conventional Commit prefixes (`ci:`, `test:`) |
| Handling updates | Merge everything at once; **merge when green, one at a time; major upgrades of related libraries together** | A failing update shows its own cause. Cucumber 8 needed JUnit 6 and an explicit JSON library, so it was done on an own branch and Dependabot's pull request was closed |
| Workflow checks | None; **actionlint locally** (in CI planned) | Catches YAML, expression and shell mistakes before pushing |
| Branch protection | Required checks (paid for private repositories); **team rule** | Merge only with green checks, from the terminal |

## Decision

One workflow, `continuous-integration.yml`, with the jobs
`build-and-test` (three platforms), `format-check`, `benchmark (smoke run)` and
`end-to-end tests (Cucumber)`, and Dependabot for GitHub Actions and Maven.

## Consequences

- A pull request shows six checks; it is merged only when all are green.
- macOS runners are the slowest and sometimes queue; fewer macOS runs and
  `timeout-minutes` on every job are planned in the quality stage.
- Major updates may need code changes; they are tested locally before merging.
