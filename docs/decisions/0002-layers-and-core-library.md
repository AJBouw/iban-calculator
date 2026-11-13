# 0002. Layers and one core library

- **Status:** Accepted
- **Date:** decided in the project setup and the m-check (PR #9, #10); recorded 2026-11-13

## Context

The program has three kinds of logic: the rules of the assignment (m-check,
splitting a range), the use cases (count, list, and search with p threads), and the
console (questions, parsing, output). Concurrency strategies are compared and
replaced during the project; the rules must not change when they do. The tests
must check the code that is actually delivered.

## Options

| Option | For | Against |
|---|---|---|
| Everything in `main.cpp` | Short | Nothing can be tested or measured in isolation |
| **Layers in one library** | Domain testable without threads or console; strategies replaceable | More files and headers |
| Separate library per layer | Dependencies enforced by the linker | More CMake for a small program |

## Decision

- Folders per layer under `include/iban/` and `src/`:
  - **domain:** m-check, `Range`, `splitRange`, `countValid`; no threads, no I/O.
  - **application:** use cases with threads (`countParallel`), the `Query`, and the input rules (`Limits.h`).
  - **infrastructure:** generic concurrency building blocks (locks, a shared counter, and starting and joining threads), introduced with the shared counter.
  - **presentation:** console input and parsing.
- Dependencies point inwards: presentation → application → domain.
- Everything except `main()` is in the static library `iban_core`; the program,
  the unit tests and the benchmark link the same library.
- One type per header; namespaces follow the folders (`iban::domain`, …).

## Consequences

- The domain is tested without threads; every parallel variant is checked
  against the sequential `countValid`.
- A new concurrency strategy is a new function in the application layer; the
  domain stays unchanged.
- The layer rule is a convention, not enforced by the build; reviews check it.