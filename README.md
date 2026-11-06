# IBAN Calculator
A multithreaded command-line tool that counts, lists or searches account numbers
satisfying the **m-check**, a weighted digit-sum check generalised from the
*Dutch 11-proef*. Built for the *Concurrent Programming* module in modern C++20.

## Table of Contents
- [Background](#background)
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
- [Testing](#testing)
- [Development Workflow](#development-workflow)
- [Status and roadmap](#status-and-roadmap)
- [Assumptions and open questions](#assumptions-and-open-questions)

---

## Background
Before IBAN, Dutch bank account numbers were validated with the *11-check*.
Each digit is multiplied by a weight based on its position (the last digit
by 1, the second-to-last by 2, and so on). The number is valid when the sum
is a multiple of 11.

Example for `23.12.43.138`:

```
2·9 + 3·8 + 1·7 + 2·6 + 4·5 + 3·4 + 1·3 + 3·2 + 8·1 = 110 = 10 · 11  ✓
```

The **m-check** generalises this: the sum must be a multiple of a given
modulus `m` (1–256). Numbers may have any number of digits.

The range of numbers to check can be very large (up to 2³¹−1), so the work
is divided fairly over `p` threads.

## Features
| Mode | `u` | Output |
|---|---|---|
| **Count** | `0` | The number of valid numbers in `[b, e)`, accumulated in a shared counter |
| **List** | `1` | Every valid number, written as soon as it is found, as `<sequence> <number>` with ascending sequence numbers |
| **Search** | `2` | Number `s` if it is in range and valid, otherwise `-1`. The other threads stop as soon as it is found |

Concurrency requirements:

- The range is split over `p` threads **without gaps or overlap**. Thread
  workloads differ by **at most one** number.
- Shared state is protected with RAII locks, or is lock-free where possible.
- Threads are reused through a thread pool.

## Getting Started

### Prerequisites

| Tool | Version | macOS | Linux (Debian/Ubuntu) | Windows |
|---|---|---|---|---|
| C++20 compiler | Apple clang 15+, GCC 12+, MSVC 2022 | `xcode-select --install` | `apt install g++` | Visual Studio 2022 (Desktop C++) |
| CMake | 3.31+ | `brew install cmake` | [cmake.org](https://cmake.org/download/) | [cmake.org](https://cmake.org/download/) |
| Ninja | any | `brew install ninja` | `apt install ninja-build` | `winget install Ninja-build.Ninja` |
| Git | any | included with Xcode tools | `apt install git` | [git-scm.com](https://git-scm.com/) |

GoogleTest is downloaded automatically on the first configuration.

### Build, test and run

```bash
git clone https://github.com/AJBouw/iban-calculator.git
cd iban-calculator

cmake --workflow --preset release   # configure + build + test
./build/release/iban                # run
```

Individual steps:

```bash
cmake --preset release              # configure
cmake --build --preset release      # build
ctest --preset release              # test
```

Use the `debug` preset instead of `release` for development and debugging.
On Windows, run the commands from a *Developer PowerShell for VS 2022* so
the MSVC compiler is found.

### IDE

The project opens directly in **CLion**, **Visual Studio 2022** and
**VS Code** (with the CMake Tools extension). All three read
`CMakePresets.json`.

## Usage

## Project Structure

## Architecture

The code follows **clean architecture**.

## Testing
```bash
ctest --preset release                 # all tests
ctest --preset release -L unit         # one level: unit | integration | e2e
```

| Level | Scope |
|---|---|
| Unit | One component in isolation (m-proef, range split, locks, thread pool, parser) |
| Integration | Use cases with the real thread pool, compared against a naive reference implementation for 1–256 threads |
| End-to-end | The compiled program run as a separate process with the assignment examples |

The CI also runs the tests under **ThreadSanitizer** (data races) and
**AddressSanitizer/UBSan** (memory errors, leaks, undefined behaviour).

## Development workflow

- **Branches (git flow):** `main` holds released versions, `develop` is the
  integration branch, and each `feature/*` branch holds one component. Every
  change reaches `develop` through a pull request.
- **CI:** every push and pull request to `main` or `develop` is built and
  tested on macOS, Linux and Windows
  (`.github/workflows/continuous-integration.yml`).
- **Commits:** [Conventional Commits](https://www.conventionalcommits.org/)
  (`feat:`, `fix:`, `test:`, `build:`, `ci:`, `docs:`, `style:`, `chore:`).
- **Code style:** enforced with `.clang-format` (`clang-format -i <file>`).
- **Comments:** `///` documents what code does and why: design decisions,
  complexity, and the requirement it implements.

## Status and roadmap

| Step | Component | Status |
|---|---|---|
| 1 | Project skeleton, CMake presets, CI | ✅ |
| 2 | Domain: m-proef and incremental walker | ⬜ |
| 3 | Domain: fair range splitting | ⬜ |
| 4 | Application: ports and use cases (count, list, search) | ⬜ |
| 5 | Infrastructure: spin lock and mutex lock | ⬜ |
| 6 | Infrastructure: thread pool | ⬜ |
| 7 | Presentation: input, options, output, end-to-end tests | ⬜ |
| 8 | Quality: sanitizers, clang-tidy, performance checks | ⬜ |
| 9 | Documentation and release 1.0.0 | ⬜ |

## Assumptions and open questions

These are being clarified with the lecturer:

1. **Leading `0` in the assignment examples.** The examples contain seven
   numbers where six are specified. It is currently interpreted as the lock
   type.
2. **Is `0` a valid number?** Its digit sum is 0, which is a multiple of
   every `m`, so it currently counts as valid.