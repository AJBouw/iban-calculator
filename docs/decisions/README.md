# Architecture decision records

Each file records one decision: the context, the options that were considered,
the choice and its consequences. A record is not rewritten when a decision
changes; a new record replaces it, so the reasoning behind every step stays
visible. The format follows Michael Nygard's
[architecture decision records](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions).

Records 0001 to 0009 and 0011 were written afterwards for decisions taken in earlier
steps; their content comes from the README, code comments and pull requests of
those steps.

| # | Decision | Status |
|---|---|---|
| [0001](0001-build-system-cmake.md) | Build system: CMake with presets | Accepted |
| [0002](0002-layers-and-core-library.md) | Layers and one core library | Accepted |
| [0003](0003-code-style-and-workflow.md) | Code style, comments and Git workflow | Accepted |
| [0004](0004-m-check.md) | m-check: arithmetic, 64-bit numbers, invalid arguments throw | Accepted |
| [0005](0005-input-handling.md) | Input: forms, questions per field, one source for the rules | Accepted |
| [0006](0006-range-split.md) | Range split: exactly p contiguous parts | Accepted |
| [0007](0007-threads-and-result-slots.md) | Threads: std::thread with explicit joins, one result slot per thread | Accepted; result slots superseded by [0010](0010-shared-counter-and-lock.md) |
| [0008](0008-measurements.md) | Measurements: Google Benchmark, wall-clock time, local runs | Accepted |
| [0009](0009-end-to-end-tests.md) | End-to-end tests: Cucumber in Java | Accepted |
| [0010](0010-shared-counter-and-lock.md) | Shared counter: local count, one locked add per thread, lock as a template parameter | Accepted |
| [0011](0011-continuous-integration-and-updates.md) | Continuous integration and dependency updates | Accepted |
| [0012](0012-list-mode-output.md) | List mode: sequence number and line under one lock | Accepted |
| [0013](0013-meaning-of-s.md) | Search mode: s is the account number to find | Accepted |
| [0014](0014-stopping-the-search.md) | Search mode: an atomic stop flag, checked before every number | Accepted |

Decisions still to be recorded when they are taken: the algorithmic
improvements (the incremental m-check and the others), the thread pool (fixed
ranges versus a shared task queue), and the release (removing the LEARN lines).

Template for a new record:

```markdown
# NNNN. Title

- **Status:** Proposed | Accepted | Superseded by NNNN
- **Date:** YYYY-MM-DD

## Context
## Options
## Decision
## Consequences
```