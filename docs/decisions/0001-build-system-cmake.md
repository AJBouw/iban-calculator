# 0001. Build system: CMake with presets

- **Status:** Accepted
- **Date:** decided in the project setup (PR #9); recorded 2026-11-13

## Context

The assignment does not prescribe a build system; one compiler call such as
`g++ -std=c++20 -pthread …`, as in the lectures, builds the program. The project
needs more: builds on macOS, Linux and Windows, a Debug build for development
and a Release build for measurements, unit tests, an optional benchmark, end-to-
end tests and a formatting check, all reproducible in CI.

## Options

| Option | For | Against |
|---|---|---|
| Single compiler command or shell script | Nothing to learn | No Windows/MSVC, no tests or presets, flags repeated everywhere |
| Makefile | Simple, widely known | Not portable to MSVC without extra tools; dependencies by hand |
| **CMake with presets** | De facto standard for C++; generates the native build for every compiler; CTest, FetchContent, `Threads::Threads`; opened directly by CLion and Visual Studio | An extra language to learn and maintain |
| Meson, Bazel | Modern, fast | Less common in C++ courses and IDEs |
| IDE project files | No extra tool | Ties the project to one IDE |

## Decision

CMake 3.31 with `CMakePresets.json` (schema version 10, for `$comment`).
Presets `debug` and `release`, one build folder per preset. Dependencies
(GoogleTest, Google Benchmark) are fetched at pinned versions with
`FetchContent` and marked `SYSTEM`, so their headers do not trigger the project's
strict warnings. Optional parts are switched with options:
`IBAN_BUILD_TESTS` (on), `IBAN_BUILD_BENCHMARKS` (off), `IBAN_BUILD_E2E_TESTS` (off).
The version number is defined once in `project()` and generated into `Version.h`.

## Consequences

- One command per task on every platform: `cmake --preset debug`,
  `cmake --build --preset debug`, `ctest --preset debug`; CI uses the same commands.
- Pinned dependency versions make builds reproducible; Dependabot proposes updates.
- Readers must know some CMake; comments in `CMakeLists.txt` explain every block.
- Background: README, section "Build system: why CMake".