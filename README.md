# IBAN Calculator

[![CI](https://github.com/AJBouw/iban-calculator/actions/workflows/continuous-integration.yml/badge.svg?branch=develop)](https://github.com/AJBouw/iban-calculator/actions/workflows/continuous-integration.yml)

A multithreaded command-line application written in modern C++20. It generalises the Dutch 11-check (11-proef) into an *m-check* and is developed for the Concurrent Programming module.

The application is designed to count, list, or search for numbers that satisfy the m-check. The implementation focuses on correctness, efficient algorithms, thread safety and measurable concurrency performance.

> **Project status:** Count mode and list mode work with `p` threads and are measured. Search mode is under development.

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

With `m = 11` the check detects every single-digit error and every swap of two adjacent digits: such an error changes the sum by `weight × difference` or by `difference`, and with weights and differences below 11 and 11 being prime, that change is never a multiple of 11. The unit tests verify this property.

Numbers may contain any number of decimal digits, subject to the input range supported by the assignment.

The application can process a large range of numbers, so the workload is distributed across multiple threads.

## Features and Requirements

The application is designed to support three operating modes:

| Mode | `u` | Intended behaviour |
|---|---:|---|
| Count | `0` | Count all valid numbers in the range `[b, e)`. |
| List | `1` | Output every valid number, together with an ascending sequence number. |
| Search | `2` | Search for the number `s`: print it when it lies in the range and passes the m-check; otherwise, print `-1`. The other threads stop as soon as possible (interpretation to be confirmed, see open question 2). |

### Input parameters

The program asks for input on the console and reads one line. The assignment describes the following parameters:

| Parameter | Description | Constraints |
|---|---|---|
| `b` | Inclusive lower bound | `0 ≤ b ≤ 2³¹ − 1` |
| `e` | Exclusive upper bound | `b < e ≤ 2³¹ − 1` |
| `m` | Modulus | `1 ≤ m ≤ 256` |
| `p` | Number of threads | `1 ≤ p ≤ 256` |
| `u` | Operating mode | `0`, `1` or `2` |
| `s` | Search parameter | Optional; required in search mode (`u = 2`) |

### Assignment requirements

- Divide the range `[b, e)` between `p` threads without gaps or overlap.
- Ensure that the workloads of the threads differ by at most one number.
- Use exactly `p` threads. When `p` exceeds the number of values, some threads receive an empty range and finish immediately.
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
| Java (JDK) | 25 | End-to-end tests only (optional) |
| Maven | 3.9 or later | End-to-end tests only (optional) |

GoogleTest 1.15.2 is downloaded automatically by CMake during the first configuration when tests are enabled. Maven downloads Cucumber and JUnit during the first run of the end-to-end tests.

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
brew install openjdk@25 maven   # only for the end-to-end tests
```

**Linux (Debian/Ubuntu)**

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git pipx
pipx install clang-format==21.1.8
sudo apt install openjdk-25-jdk maven   # only for the end-to-end tests
```

**Windows**

Install Visual Studio 2022 with the *Desktop development with C++* workload, CMake, and Ninja. Git can be installed from [git-scm.com](https://git-scm.com/). For the end-to-end tests, install JDK 25 (for example, [Eclipse Temurin](https://adoptium.net/)) and [Maven](https://maven.apache.org/download.cgi).

### Build, test and run

Clone the repository:

```bash
git clone https://github.com/AJBouw/iban-calculator.git
cd iban-calculator
```

Configure, build, and test the Release configuration:

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

The workflow preset combines configuration, compilation, and testing. These steps can also be executed separately:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Use the Debug configuration during development and debugging. Use Release for performance measurements and final evaluation.

On Windows, run the commands from a Developer PowerShell for Visual Studio 2022 so that the MSVC compiler toolchain is available.

## Usage

Start the program. It asks for each value in turn and explains what it means and which values are allowed:

```text
IBAN Calculator 0.1.0
Enter each value and press Enter, or paste all values on one line.
The assignment examples start with an extra 0: use it only on one line.
  b  lower bound, inclusive    (0 to 2147483647): 274856170
  e  upper bound, exclusive    (greater than b, at most 2147483647): 274856190
  m  modulus of the m-check    (1 to 256): 11
  p  number of threads         (1 to 256): 4
  u  mode                      (0 = count, 1 = list, 2 = search): 0
  s  number to search for      (required when u = 2, press Enter to skip):
2
```

An empty answer for a required value repeats the question; an empty answer for `s` skips it.

### All values on one line

When the first answer contains more than one value, it is taken as the complete input. This keeps the examples from the assignment usable, and allows the input to be piped:

```bash
echo "0 274856170 274856190 11 4 0 0" | ./build/release/iban
./build/release/iban < input.txt
```

Three forms are accepted on one line, so that both the written specification and the examples of the assignment work:

| Form | Fields | Source |
|---|---|---|
| 6 numbers | `b e m p u s` | The written specification. |
| 5 numbers | `b e m p u` | The same specification: `s` is optional and only needed in search mode. |
| 7 numbers | `0 b e m p u s` | The examples in the assignment start with an extra `0`. It is treated as an optional leading zero: it must be `0` and is not used (see assumption 1). |

Examples from the assignment:

```text
0 274856170 274856190 11 4 0 0     # count mode, expected output: 2
0 274856170 274856190 11 4 1 0     # list mode
0 374856170 374856250 12 4 1 0     # list mode with m = 12
```

### Output and errors

- **Standard output** contains only results. The questions and error messages are written to **standard error**, so the results can be compared or redirected (`iban > out.txt`) without the prompt.
- Invalid input is reported with the program name and the first invalid field, for example `iban: error: m must be between 1 and 256`, and the program exits with status code 1.
- **Count mode** prints one number: how many values in `[b, e)` pass the m-check.
- **List mode** prints one line per valid number: the sequence number, a space, and the number, for example `1 274856182`. The numbers are written as soon as a thread finds them, so their order may differ per run; the sequence numbers always ascend from 1 in the order of the lines.
- Until search mode is implemented, the program prints `input OK:` followed by the validated values for that mode.

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
│   └── main.cpp            # Entry point only
├── benchmark/
│   ├── CountBenchmark.cpp          # Google Benchmark: count mode with 1, 2, 4, ... threads
│   ├── CountVariantsBenchmark.cpp  # Shared counter, locks, atomic, and result slots compared
│   └── ListVariantsBenchmark.cpp   # List mode: one lock per line versus a writer thread
├── docs/
│   ├── decisions/          # Architecture decision records: options, choices and reasons
│   ├── data/               # Raw benchmark results (JSON)
│   ├── figures/            # Figures drawn from docs/data
│   └── measurements.md     # Method and results of the measurements
├── include/
│   └── iban/
│       ├── application/
│       │   ├── CountVariants.h # Ways to combine the counts of p threads, for measurement
│       │   ├── Limits.h        # Input rules from the assignment (b, e, m, p)
│       │   ├── ListVariants.h  # Ways to write the list from p threads, for measurement
│       │   ├── ParallelCount.h # Count mode with p threads and a shared counter
│       │   ├── ParallelList.h  # List mode with p threads and numbered lines
│       │   ├── Mode.h          # Operating mode u
│       │   └── Query.h         # A validated request
│       ├── domain/
│       │   ├── Count.h         # Sequential count of a range
│       │   ├── ForEachValid.h  # Calls a function for every valid number in a range
│       │   ├── MCheck.h        # Public interface of the m-check
│       │   ├── Range.h         # Half-open range [begin, end)
│       │   └── RangeSplit.h    # Fair split of a range into p parts
│       ├── infrastructure/
│       │   ├── BasicLockable.h # Concept for a lock type (lock and unlock)
│       │   ├── BoundedBuffer.h # Producer-consumer queue: mutex and two condition variables
│       │   ├── ForkJoin.h      # Starts one thread per part and joins them all
│       │   ├── SequencedWriter.h # Numbered lines: sequence number and line under one lock
│       │   ├── SharedCounter.h # Counter protected by a lock, the lock as a template parameter
│       │   └── SpinLock.h      # Lock that waits actively, to compare with std::mutex
│       └── presentation/
│           ├── ConsoleInput.h  # Asks for the input values
│           └── InputParser.h   # Turns text into a validated Query
├── src/
│   ├── application/
│   │   ├── CountVariants.cpp
│   │   ├── ListVariants.cpp
│   │   ├── ParallelCount.cpp
│   │   └── ParallelList.cpp
│   ├── domain/
│   │   ├── Count.cpp
│   │   ├── MCheck.cpp          # Reference implementation of the m-check
│   │   └── RangeSplit.cpp
│   └── presentation/
│       ├── ConsoleInput.cpp
│       └── InputParser.cpp
├── tests/
│   ├── e2e/                # End-to-end tests with Cucumber (Java, Maven)
│   │   ├── pom.xml
│   │   └── src/test/
│   │       ├── java/nl/iban/e2e/
│   │       │   ├── IbanSteps.java        # Step definitions: start the program, check output
│   │       │   └── RunCucumberTest.java  # Runs the scenarios on the JUnit Platform
│   │       └── resources/
│   │           ├── features/             # Scenarios in Gherkin (*.feature)
│   │           └── junit-platform.properties
│   ├── race/
│   │   ├── ListOrderDemo.cpp # Shows a race condition without a data race (list order)
│   │   └── RaceDemo.cpp    # Shows the data race of a counter without a lock
│   └── unit/
│       ├── SetupTest.cpp
│       ├── MCheckTest.cpp
│       ├── InputParserTest.cpp
│       ├── ConsoleInputTest.cpp
│       ├── RangeSplitTest.cpp
│       ├── CountTest.cpp
│       ├── ParallelCountTest.cpp
│       ├── SharedCounterTest.cpp
│       ├── CountVariantsTest.cpp
│       ├── ForEachValidTest.cpp
│       ├── BoundedBufferTest.cpp
│       ├── SequencedWriterTest.cpp
│       └── ListVariantsTest.cpp
├── tools/
│   ├── plot_speedup.py     # Draws the speedup figure from benchmark JSON
│   └── requirements.txt    # Pinned Python packages for tools/
└── build/                  # Generated; not committed
```

Public headers live under `include/iban/<layer>/` and implementation files under `src/<layer>/`, one folder per layer. Namespaces follow the folders (`iban::domain`, `iban::application`, `iban::presentation`). Generated files and build artefacts are kept outside the source directories.

## Architecture and Design

The intended design separates the application into responsibilities:

- **Domain:** m-check validation and range-splitting algorithms.
- **Application:** count, list and search use cases, coordinating the required operations.
- **Infrastructure:** thread management, synchronisation and other technical implementations.
- **Presentation:** input parsing, parameter validation, and console output.

The design aims to keep domain logic independent of threading and console input/output. This makes the algorithms easier to test in isolation and allows different concurrency strategies to be evaluated without changing the underlying validation rules.

The reasons behind these and later choices, with the options that were considered, are recorded in [`docs/decisions/`](docs/decisions/README.md).

All logic except `main()` lives in the static library `iban_core`. Both the program and the unit tests link it, so the tests exercise exactly the code that ships. Only `include/` is exposed to other targets.

### m-check

`weightedDigitSum` computes the sum arithmetically (`% 10` and `/ 10`), without converting the number to text: no memory allocation and O(d) steps for a number with d digits. `passesMCheck` adds the divisibility check.

- **Invalid arguments throw** `std::invalid_argument`: a modulus below 1 or a negative number can only come from a programming error, and returning `false` would silently produce a wrong result. `x % 0` is undefined behaviour in C++, so a modulus of 0 must never reach the division.
- **`std::int64_t`** is used for numbers, leaving headroom for calculations with the bounds and intermediate results.
- **Thread-safe** by design: both functions use only their arguments and local variables, so concurrent calls share no mutable state.
- This implementation is deliberately simple. Faster variants are verified against it.

### Range split

`splitRange` divides the range `[b, e)` into exactly `p` contiguous parts. With `n = e − b` values, every part gets `q = n / p` values and the first `r = n % p` parts get one extra, so part sizes differ by at most one. Each part starts where the previous one ends, so the parts cover the range without gaps or overlap.

```text
[0, 10) into 4 parts:  q = 2, r = 2  →  [0, 3) [3, 6) [6, 8) [8, 10)
```

- **Half-open ranges** (`Range{begin, end}`) make adjacent parts share a boundary without overlap, matching `[b, e)` in the assignment.
- **Contiguous blocks instead of interleaving** (`x, x + p, x + 2p, …`): each thread works on consecutive numbers, which an incremental m-check can reuse and which keeps memory access local.
- **Always exactly `p` parts.** When `p > n`, the last parts are empty; each thread still gets a part, as the assignment requires `p` threads.
- **Domain layer:** the split is pure arithmetic and does not know about threads. Its cost is O(p), independent of the size of the range.

### Parallel count

`countParallel` splits `[b, e)` into `p` parts and starts one thread per part. Each thread calls `countValid` (the sequential count from the domain layer) on its own part, and then adds its count **once** to a **shared counter** protected by a `std::mutex`, as the assignment requires.

```text
splitRange([b, e), p)  →  part 0   part 1   …   part p−1
                            │        │             │
                         thread 0 thread 1  …  thread p−1
                     count locally, then lock, add, unlock (p times in total)
                            │        │             │
                            └──── SharedCounter<std::mutex> ────┘  →  value after join
```

- **The lock is taken only `p` times**: the counting itself uses only local data, so the threads do not wait for each other while they count. A lock per valid number would also be correct, but every thread would wait for the same lock; the benchmark measures both (`docs/measurements.md`).
- **The lock is a template parameter** (`SharedCounter<Lock>`, constrained by the C++20 concept `BasicLockable`), so `lock()` and `unlock()` can be inlined, and `SpinLock` can be measured with the same code. The options and the reasons are in [decision 0010](docs/decisions/0010-shared-counter-and-lock.md).
- **RAII locking**: `std::lock_guard` releases the lock also when an exception is thrown.
- **`std::thread` with explicit joins**, as in the lectures. If starting a thread fails, the threads already running are joined before the error is passed on; otherwise `std::thread`'s destructor would call `std::terminate`. C++20 `std::jthread` would do this automatically (it joins in its destructor) and also offers a `std::stop_token`; the code explains this in comments.
- **Exactly `p` threads**: a thread with an empty part finishes immediately.
- **Starting and joining** is in one helper, `runInParallel` (`ForkJoin.h`), so every variant in `CountVariants.h` uses the same exception-safe thread handling.

### Parallel list

`listParallel` splits the range like the count and starts one thread per part. Every thread walks its part with `forEachValid` and writes each valid number at once through one shared `SequencedWriter`, which holds the output stream, a `std::mutex`, and the next sequence number.

```text
thread i finds x  →  text " x\n" (outside the lock)  →  lock: sequence number + 1, write "<n> x\n", unlock
```

- **Sequence number and line in one critical section.** If the number were taken first (even with an atomic) and the line written afterwards, another thread could write a higher number in between. `ListOrderDemo` shows this: hundreds of lines out of order per run, while ThreadSanitizer reports nothing, because every single access is synchronised. It is a race condition, not a data race.
- **As little as possible under the lock**: the account number is formatted before the lock is taken.
- **No flush per line**: on a terminal the lines appear one by one anyway; flushing every line would make large lists much slower when the output is piped.
- **A writer thread with a bounded buffer** (producer-consumer, as in the lectures) is the alternative: the counting threads push numbers into a `BoundedBuffer`, and one extra thread numbers and writes them. It is implemented and measured (`listWithWriterThread`). The reasons for the choice are in [decision 0012](docs/decisions/0012-list-mode-output.md).
- Writing is sequential by nature, so list mode cannot scale like count mode when many numbers are valid.

### Input handling

Input passes through two layers before any calculation starts:

1. **`ConsoleInput`** (presentation) asks for the values and joins them into one line. It does not validate.
2. **`InputParser`** (presentation) converts the line into a `Query` in two phases: text to numbers with `std::from_chars`, which detects trailing characters such as `12abc` and overflow, then numbers to a `Query`, checked in input order so that the message names the first invalid field.

Design choices:

- **Invalid input is returned, not thrown.** A typing error by the user is expected behaviour, unlike an invalid argument inside the program (see the m-check). The parser returns a `ParseResult` with either a valid `Query` or a message; `std::expected` would express this directly but requires C++23.
- **One source for every input rule.** `application/Limits.h` holds all bounds from the assignment. The parser, the help text, the error messages, the `Query` defaults and the tests are built from these constants, so they cannot disagree.
- **A `Query` is always valid.** Only the parser creates one, so the rest of the program does not repeat the checks. `s` is a `std::optional`, because any number, including `-1` or `0`, could be real input.
- **`Mode` is an `enum class`**, so a mode cannot be mixed up with an integer and the compiler warns when a `switch` misses a mode.

The version number is defined once, in `project()` in `CMakeLists.txt`; CMake generates a header from `cmake/Version.h.in`, so the code never repeats it.

The final architecture and its dependency directions will be documented as the implementation evolves.

## Testing

The project uses GoogleTest for the unit tests and Cucumber for the end-to-end tests.

Run all tests:

```bash
ctest --preset debug
```

Run tests with a specific label:

```bash
ctest --preset debug -L unit
```

### Current unit tests

| Suite | Verifies |
|---|---|
| `Setup` | The compiler uses C++20 and threads can be started. |
| `WeightedDigitSumTest` | Hand-calculated sums, including 0, 2³¹ − 1 and the largest 10-digit sum. |
| `PassesMCheck` | The assignment example, error detection with `m = 11`, and rejection of invalid arguments. |
| `InputParser` | The assignment examples, the three input forms, the boundary values, and one test per validation rule. |
| `ConsoleInput` | One value per question, a pasted complete line, skipping `s`, repeated questions, and end of input. |
| `CountValid` | The assignment examples (2 and 6), an empty range, `m = 1` and an invalid modulus. |
| `ParallelCount` | The parallel count equals the sequential count for `p` = 1 … 16 and several moduli, also with more threads than numbers and with 256 threads. |
| `SharedCounterTest` | With `std::mutex` and with `SpinLock`: start value, additions, and no lost addition when 16 threads add at the same time; the concept accepts both locks and rejects a type without `lock()`/`unlock()`. |
| `RunInParallel` | Every index runs exactly once. |
| `CountVariants` | Every way of combining the counts (result slots, atomic, mutex and spin lock per number and per thread) equals the sequential count for many thread counts and moduli. |
| `ForEachValid` | Visits the assignment examples in ascending order and exactly the numbers that `countValid` counts. |
| `BoundedBuffer` | First in, first out; remaining items after `close()` and then none; `close()` wakes a waiting consumer; 8 producers and 4 consumers with a buffer of 1 deliver every item exactly once. |
| `SequencedWriterTest` | With `std::mutex` and with `SpinLock`: numbered lines, and ascending sequence numbers when 16 threads write 32,000 lines at the same time. |
| `ListVariants` | Every way of writing the list (lock, spin lock, writer thread with a buffer of 1024 and of 1) writes each valid number once, as `<n> <number>` with sequence numbers 1, 2, 3, …, for many thread counts and moduli. |
| `RangeSplit` | The assignment example, the remainder distribution, more parts than values, and the three requirements (no gaps, no overlap, sizes differ by at most one) for thousands of combinations, including the largest range. |

### Planned test levels

| Level | Scope |
|---|---|
| Unit | Individual components, such as m-check validation, range splitting, synchronisation and input parsing. |
| Integration | Complete use cases using the actual thread-management implementation. Results are compared with a simple reference implementation. |
| End-to-end | The compiled application is executed as a separate process and tested against the assignment examples (in place, see below). |

### End-to-end tests

The end-to-end tests describe the behaviour that a user sees, in [Gherkin](https://cucumber.io/docs/gherkin/reference/): given some input, the program prints this output and ends with this exit code. [Cucumber](https://cucumber.io/docs/cucumber/) (Java) starts the compiled program as a separate process for every scenario, writes the input to its standard input and checks its standard output, standard error, and exit code. The tests therefore know nothing about the C++ code; they test the program exactly as it is delivered.

They need Java 25 and Maven, so they are off by default:

```bash
cmake --preset debug -DIBAN_BUILD_E2E_TESTS=ON
cmake --build --preset debug
ctest --preset debug -L e2e --output-on-failure
```

CTest starts Maven with the path to the built program. The first run downloads Cucumber and JUnit. Use `-V` to see every step, and open `tests/e2e/target/cucumber-report.html` for the report.

Current scenarios (41 in total):

| Feature | Verifies |
|---|---|
| `count_mode.feature` | The assignment examples, a single number valid (1) or invalid (0), the three one-line forms and input per question, and the same result for `p` = 1, 2, 3, 8, 20 and 256 (an end-to-end check of the parallel count). |
| `list_mode.feature` | Both list examples from the assignment, a range without valid numbers, and the same list for `p` = 1, 2, 3, 20 and 256; the numbers are compared as a set, and the sequence numbers must ascend from 1. |
| `input_errors.feature` | No input, missing values, and one example per validation rule (b, e, m, p, u, s, invalid numbers): message on standard error, no output, exit code 1. |

The assignment examples are tagged `@assignment`; run only those with `mvn -f tests/e2e/pom.xml test -Diban.executable=<path to iban> -Dcucumber.filter.tags=@assignment`.

Scenarios are written in `tests/e2e/src/test/resources/features/`. These steps are available:

| Step | Meaning |
|---|---|
| `Given the input "…"` | One line of input, for example, all values on one line |
| `Given the input lines:` (with a `"""` block) | Several lines of input, one answer per question |
| `Given no input` | Standard input is closed immediately |
| `When the program runs` | Starts the program (time limit 30 seconds) |
| `Then the output is "…"` | Standard output equals this single line |
| `Then the output is:` (with a `"""` block) | Standard output equals these lines |
| `Then the program counts 2 valid account numbers` | Count mode: standard output is this number (also `1 valid account number`) |
| `Then the program lists these valid account numbers:` (with a one-column table) | List mode: every line is two numbers, and the account numbers equal the table, in any order |
| `Then the sequence numbers ascend from 1` | List mode: the first numbers of the lines are 1, 2, 3, … in the order of the lines |
| `Then the program lists no account numbers` | List mode: nothing on standard output |
| `Then the output is empty` | Nothing on standard output |
| `Then the program succeeds` | Exit code 0 |
| `Then the program fails` | Exit code other than 0 |
| `Then the error message contains "…"` | Standard error contains this text |

### Concurrency testing

Testing will cover different thread counts, including the boundary values supported by the assignment.

Additional validation will focus on:

- Correctness for one and multiple threads.
- Data races and synchronisation errors.
- Correct range distribution.
- Early termination in search mode.
- Consistent output and sequence numbering.

Performance measurements are taken with Release builds only; Debug builds are not optimised and would not give representative results. The benchmark and its results are described in [`docs/measurements.md`](docs/measurements.md):

```bash
cmake --preset release -DIBAN_BUILD_BENCHMARKS=ON
cmake --build --preset release
./build/release/iban_benchmark --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
```

The benchmark uses [Google Benchmark](https://github.com/google/benchmark) (v1.10.0, fetched by CMake like GoogleTest). It is off by default, so normal builds and CI do not download it. The method, the results and the sources used are in [`docs/measurements.md`](docs/measurements.md#references).

#### Data races and ThreadSanitizer

A data race cannot be tested reliably with a normal test: a counter without a lock sometimes gives the right result by chance. Two tools show it instead:

- **`iban_race_demo`** counts with a shared counter without a lock (`NoLock`) and prints how many increments were lost, next to the same count with `std::mutex` and `SpinLock`.
- **`iban_list_order_demo`** writes numbered lines with the sequence number and the line in two separate steps, and counts the lines that are out of order. ThreadSanitizer reports nothing here: it is a race condition without a data race, which only the right critical section prevents.
- **ThreadSanitizer** (`tsan` preset) reports every data race while the program runs, with the lines of the two accesses. It reports the race in the demo, and none in the unit tests or the program.

```bash
./build/debug/iban_race_demo                  # lost updates without a lock

cmake --preset tsan                           # Clang or GCC; not MSVC
cmake --build --preset tsan
./build/tsan/iban_race_demo                   # "WARNING: ThreadSanitizer: data race"
./build/tsan/iban_list_order_demo             # lines out of order, but no warning
ctest --preset tsan                           # all unit tests, no race reported
```

The variants are compared with a second benchmark program:

```bash
./build/release/iban_benchmark_variants --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
```

A sanitizer job in CI and static analysis follow in the quality stage.

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
| `benchmark (smoke run)` | Build the benchmark and run each thread count once; results on the run's Summary page, not used as measurements |
| `end-to-end tests (Cucumber)` | Run the Cucumber scenarios against a Release build on Linux; the HTML report is kept as the artifact `cucumber-report` |

A pull request is merged only when all checks pass. Dependabot keeps the GitHub Actions versions and the Maven dependencies of the end-to-end tests up to date.

### Build system: why CMake

The assignment does not prescribe a build system; a single command such as `g++ -std=c++20 -pthread …` (as in the lectures) would compile the program. CMake is used because the project needs more than one compiler call:

| Need | How CMake provides it |
|---|---|
| Build on macOS, Linux and Windows (Clang, GCC, MSVC) | One description generates the native build for each compiler; CI uses it on all three platforms. |
| Portable thread support | `Threads::Threads` adds `-pthread` where a platform needs it, instead of a compiler-specific flag. |
| Debug for development, Release for measurements | Presets (`debug`, `release`) give two separate builds; measurements are only valid in Release. |
| Unit tests in the same build | GoogleTest is fetched at a pinned version and every test is registered with CTest. |
| Optional benchmark | Google Benchmark is fetched only when `IBAN_BUILD_BENCHMARKS` is on. |
| One library for program and tests | `iban_core` is compiled once and linked by both, so the tests check the shipped code. |
| Formatting check | `format` and `format-check` targets use the pinned clang-format version. |
| Version number in one place | `project(VERSION …)` generates `Version.h`. |
| IDE support | CLion and Visual Studio open the project directly from `CMakeLists.txt`. |

Trade-off: CMake is an extra language to learn and maintain. Alternatives were considered: a Makefile does not work with MSVC without extra tools, Meson and Bazel are less common in C++ courses, and IDE project files tie the project to one IDE. CMake is the de facto standard for C++ projects.

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

Comments should explain design decisions, invariants, complexity, and relevant requirements rather than simply restating what the code does.

## Status and Roadmap

Each step adds one concern, measured against the previous step where relevant, so the effect of every change can be explained.

| Step | Component | Status |
|---:|---|:---:|
| 0 | Project setup: CMake presets, version header, smoke tests, formatting, CI | ✅ |
| 1 | m-check: reference implementation in the core library | ✅ |
| 2 | Input: asking per field, parsing and validation | ✅ |
| 3 | Fair range splitting | ✅ |
| 4 | Count mode: sequential baseline compared with `p` threads, benchmark | ✅ |
| 4b | End-to-end tests of the assignment examples in Gherkin (Cucumber, Java) | ✅ |
| 5 | Shared counter: data race, mutex, local counting, spin lock, ThreadSanitizer | ✅ |
| 6 | List mode: sequence number and line under one lock, bounded buffer with a writer thread | ✅ |
| 7 | Search mode with atomic stop flag and early termination | ⬜ |
| 8 | Performance comparison, including an incremental m-check walker | ⬜ |
| 9 | Thread pool | ⬜ |
| 10 | Quality checks (sanitizers, static analysis, Doxygen) and release 1.0.0 | ⬜ |

✅ done · 🔄 in progress · ⬜ planned

## Assumptions and Open Questions

The following points still need to be clarified against the original assignment or with the lecturer:

1. **Input format (assumption):** The assignment examples contain seven values, while the written description specifies five required parameters and one optional parameter. Every example starts with an extra `0`. This is treated as an optional leading zero: the program accepts it, requires it to be exactly `0`, and does not use it. Any other first value in a 7-number line is rejected, so a mistyped line is reported instead of silently misread. To be confirmed with the lecturer.

2. **Search parameter `s`:** The assignment text says the program must find "a valid account number", and print `-1` when "the account number" is not found. Three interpretations are possible:
   - **A (current choice):** `s` is the number to search for; the threads search the range, and the thread that finds `s` stops the others. Weakness: the answer could also be computed directly, without searching.
   - **B:** `s` is not used; the program searches for any valid number in the range.
   - **C:** `s` is a starting point for the search.

   In every interpretation one thread must stop the others promptly, which is the concurrency requirement this mode tests.

3. **Zero as a valid number:** The weighted digit sum of zero is zero. Since zero is divisible by every positive modulus, `passesMCheck(0, m)` returns `true`. Whether zero should be counted as a valid account number will be checked against the assignment requirements.

4. **Thread pool and synchronisation:** The exact implementation strategy will be determined during development and evaluated against correctness, performance, and the concurrency assessment criteria.

These assumptions will be updated when the assignment interpretation has been confirmed.