# IBAN Calculator

[![CI](https://github.com/AJBouw/iban-calculator/actions/workflows/continuous-integration.yml/badge.svg?branch=develop)](https://github.com/AJBouw/iban-calculator/actions/workflows/continuous-integration.yml)

A multithreaded command-line application written in modern C++20. It generalises the Dutch 11-check (11-proef) into an *m-check* and is developed for the Concurrent Programming module.

The application is designed to count, list or search numbers that satisfy the m-check. The implementation focuses on correctness, efficient algorithms, thread safety and measurable concurrency performance.

> **Project status:** The initial project setup is complete. The CMake build configuration, C++20 toolchain checks, initial GoogleTest smoke tests, code formatting checks and continuous integration are in place. The application functionality is under development.

## Table of Contents

- [Background](#background)
- [Features and Requirements](#features-and-requirements)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Architecture and Design](#architecture-and-design)
- [Testing](#testing)
- [Development Workflow](#development-workflow)
- [Status and Roadmap](#status-and-roadmap)
- [Assumptions and Open Questions](#assumptions-and-open-questions)

---

## Background

Before IBAN was introduced, Dutch bank account numbers were validated using the **11-check (11-proef)**.

Each decimal digit is multiplied by a weight determined by its position, counting from right to left. The rightmost digit has weight 1, the digit to its left has weight 2, and so on.

A number is valid if the resulting weighted sum is divisible by 11.

For example, the number `23.12.43.138`:

```text
2 × 9 + 3 × 8 + 1 × 7 + 2 × 6 + 4 × 5
+ 3 × 4 + 1 × 3 + 3 × 2 + 8 × 1
= 110
= 10 × 11
```

The **m-check** generalises this principle: instead of always checking divisibility by 11, the application accepts a configurable modulus `m` (1–256).

Numbers may contain any number of decimal digits, subject to the input range supported by the assignment.

The application can process a large range of numbers, so the workload is distributed across multiple threads.

## Features and Requirements

The application is designed to support three operating modes:

| Mode | `u` | Intended behaviour |
|---|---:|---|
| Count | `0` | Count all valid numbers in the range `[b, e)`. |
| List | `1` | Output every valid number, together with an ascending sequence number. |
| Search | `2` | Find a valid number in the range and stop further work as soon as possible. |

### Input parameters

The assignment describes the following parameters:

| Parameter | Description | Constraints |
|---|---|---|
| `b` | Inclusive lower bound | `0 ≤ b ≤ 2³¹ − 1` |
| `e` | Exclusive upper bound | `b < e ≤ 2³¹ − 1` |
| `m` | Modulus | `1 ≤ m ≤ 256` |
| `p` | Number of threads | `1 ≤ p ≤ 256` |
| `u` | Operating mode | `0`, `1` or `2` |
| `s` | Additional search parameter | Meaning to be confirmed |

### Assignment requirements

- Divide the range `[b, e)` between `p` threads without gaps or overlap.
- Ensure that the workloads of the threads differ by at most one number.
- Count mode: keep the result in a shared counter.
- List mode: write each valid number immediately, with ascending sequence numbers.
- Search mode: the other threads stop promptly after a result is found; print `-1` if nothing is found.

The list mode does not guarantee that numbers are printed in numerical order. However, sequence numbers must increase in the order in which results are written.

### Quality goals (assessment criteria)

- Protect shared mutable state against data races.
- Hold locks for as little time as possible, with exception-safe (RAII) locking.
- Reuse threads where appropriate rather than repeatedly creating and destroying them.
- Measure the effect of each optimisation.

## Getting Started

### Prerequisites

| Tool | Version | Purpose |
|---|---|---|
| C++ compiler | C++20 support | Compile the application |
| CMake | 3.31 or later | Configure and build the project (required by `CMakePresets.json`) |
| Ninja | Any supported version | Build tool used by the presets |
| Git | Any supported version | Version control |
| clang-format | 21.1.8 (exact) | Formatting checks (optional for building) |

GoogleTest 1.15.2 is downloaded automatically by CMake during the first configuration when tests are enabled.

Suggested compiler versions:

- Apple Clang 15 or later
- GCC 12 or later
- MSVC 2022

### Installation

**macOS**

```bash
xcode-select --install
brew install cmake ninja pipx
pipx install clang-format==21.1.8
```

**Linux (Debian/Ubuntu)**

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git pipx
pipx install clang-format==21.1.8
```

**Windows**

Install Visual Studio 2022 with the *Desktop development with C++* workload, CMake and Ninja. Git can be installed from [git-scm.com](https://git-scm.com/).

### Build, test and run

Clone the repository:

```bash
git clone https://github.com/AJBouw/iban-calculator.git
cd iban-calculator
```

Configure, build and test the Release configuration:

```bash
cmake --workflow --preset release
```

The Debug configuration can be used during development:

```bash
cmake --workflow --preset debug
```

Run the application:

**macOS / Linux**

```bash
./build/release/iban
```

**Windows**

```powershell
.\build\release\iban.exe
```

### Individual build steps

The workflow preset combines configuration, compilation and testing. These steps can also be executed separately:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Use the Debug configuration during development and debugging. Use Release for performance measurements and final evaluation.

On Windows, run the commands from a Developer PowerShell for Visual Studio 2022 so that the MSVC compiler toolchain is available.

## Usage

The application currently has no completed user-facing input interface. The command-line input format and examples will be documented here once the input parser and operating modes have been implemented and verified.

The expected interface is based on the assignment parameters described above.

## Project Structure

The project uses a separation between application entry point, reusable implementation code, public interfaces and tests.

Current structure:

```text
iban-calculator/
├── .github/
│   ├── dependabot.yml
│   ├── pull_request_template.md
│   └── workflows/
│       └── continuous-integration.yml
├── .editorconfig
├── .clang-format
├── .gitignore
├── CMakeLists.txt
├── CMakePresets.json
├── cmake/
│   └── Version.h.in
├── app/
│   └── main.cpp
├── tests/
│   └── unit/
│       └── SetupTest.cpp
└── build/                  # Generated; not committed
```

Public headers will be added under `include/iban/` and implementation files under `src/`, one folder per layer. Generated files and build artefacts are kept outside the source directories.

## Architecture and Design

The intended design separates the application into responsibilities:

- **Domain:** m-check validation and range-splitting algorithms.
- **Application:** count, list and search use cases, coordinating the required operations.
- **Infrastructure:** thread management, synchronisation and other technical implementations.
- **Presentation:** input parsing, parameter validation and console output.

The design aims to keep domain logic independent of threading and console input/output. This makes the algorithms easier to test in isolation and allows different concurrency strategies to be evaluated without changing the underlying validation rules.

The version number is defined once, in `project()` in `CMakeLists.txt`; CMake generates a header from `cmake/Version.h.in`, so the code never repeats it.

The final architecture and its dependency directions will be documented as the implementation evolves.

## Testing

The project uses GoogleTest for automated testing.

Run all tests:

```bash
ctest --preset debug
```

Run tests with a specific label:

```bash
ctest --preset debug -L unit
```

### Planned test levels

| Level | Scope |
|---|---|
| Unit | Individual components, such as m-check validation, range splitting, synchronisation and input parsing. |
| Integration | Complete use cases using the actual thread-management implementation. Results are compared with a simple reference implementation. |
| End-to-end | The compiled application is executed as a separate process and tested against the assignment examples. |

### Concurrency testing

Testing will cover different thread counts, including the boundary values supported by the assignment.

Additional validation will focus on:

- Correctness for one and multiple threads.
- Data races and synchronisation errors.
- Correct range distribution.
- Early termination in search mode.
- Consistent output and sequence numbering.

Performance measurements are taken with Release builds only; Debug builds are not optimised and would not give representative results.

Sanitizers and additional static-analysis tools will be added as part of the quality and verification stage.

## Development Workflow

### Branching strategy

The repository uses a Git Flow-inspired branching strategy:

- `main` contains released versions.
- `develop` is the integration branch.
- `feature/*` branches contain individual features or components.
- Changes are merged into `develop` through pull requests.

### Continuous integration

Every push and pull request to `main` or `develop` runs `.github/workflows/continuous-integration.yml`:

| Check | Purpose |
|---|---|
| `build-and-test (ubuntu-latest)` | Build and test with GCC |
| `build-and-test (macos-latest)` | Build and test with Apple Clang |
| `build-and-test (windows-latest)` | Build and test with MSVC |
| `format-check` | Verify formatting against `.clang-format` |

All four checks are required before a pull request can be merged. Dependabot keeps the GitHub Actions versions up to date.

### Commits

Commit messages follow [Conventional Commits](https://www.conventionalcommits.org/).

Examples:

```text
feat: implement m-check validation
test: add m-check boundary tests
fix: correct range distribution
build: configure GoogleTest
docs: update project architecture
chore: configure project structure
```

### Code style

The project uses:

- `.editorconfig` for shared editor and whitespace settings.
- `.clang-format` for consistent C++ formatting (Google style, 4 spaces).
- C++20 as the required language standard.
- Strict compiler warnings to identify potential problems.

Formatting is enforced through CMake targets, so it does not depend on IDE settings. clang-format is pinned to version 21.1.8 for developers and CI, because each release formats slightly differently.

Format all sources:

```bash
cmake --build --preset debug --target format
```

Check formatting without modifying files (as in CI):

```bash
cmake --build --preset debug --target format-check
```

### Documentation

Public interfaces and non-obvious implementation decisions are documented using Doxygen-style comments.

Comments should explain design decisions, invariants, complexity and relevant requirements rather than simply restating what the code does.

## Status and Roadmap

Each step adds one concern, measured against the previous step where relevant, so the effect of every change can be explained.

| Step | Component | Status |
|---:|---|:---:|
| 0 | Project setup: CMake presets, version header, smoke tests, formatting, CI | ✅ |
| 1 | m-check: reference implementation, then incremental walker | ⬜ |
| 2 | Input parsing and validation | ⬜ |
| 3 | Fair range splitting | ⬜ |
| 4 | Sequential baseline compared with `p` threads | ⬜ |
| 5 | Shared counter: data race, mutex, local counting | ⬜ |
| 6 | List mode | ⬜ |
| 7 | Search mode with atomic stop flag and early termination | ⬜ |
| 8 | Performance comparison | ⬜ |
| 9 | Thread pool | ⬜ |
| 10 | Quality checks (sanitizers, static analysis, Doxygen) and release 1.0.0 | ⬜ |

## Assumptions and Open Questions

The following points still need to be clarified against the original assignment or with the lecturer:

1. **Input format:** The assignment examples contain seven values, while the written description specifies five required parameters and one optional parameter. The additional leading value is provisionally interpreted as a lock-type selector, but this has not yet been confirmed.

2. **Search parameter `s`:** The exact meaning of `s` in search mode needs clarification. It is not yet established whether it identifies a specific number to validate or defines a starting point for searching.

3. **Zero as a valid number:** The weighted digit sum of zero is zero. Since zero is divisible by every positive modulus, the mathematical definition considers zero valid. This interpretation will be checked against the assignment requirements.

4. **Thread pool and synchronisation:** The exact implementation strategy will be determined during development and evaluated against correctness, performance and the concurrency assessment criteria.

These assumptions will be updated when the assignment interpretation has been confirmed.