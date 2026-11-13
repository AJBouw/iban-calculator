# 0009. End-to-end tests: Cucumber in Java

- **Status:** Accepted
- **Date:** decided in the end-to-end test steps (feature/e2e-cucumber, test/cucumber-8, test/e2e-scenarios); recorded 2026-11-13

## Context

The unit tests check functions; nothing checked the delivered program as a
user sees it: input on standard input, the result on standard output, messages
on standard error and the exit code. The scenarios should read as the
requirements of the assignment, written by the student.

## Options

| Option | For | Against |
|---|---|---|
| Shell scripts in CTest | No extra tools | Hard to read; no report |
| Robot Framework | Known from other work | Python stack; keyword syntax rather than Gherkin |
| Cucumber with JavaScript | Known from frontend tests | Node stack next to C++; less common for console programs |
| **Cucumber with Java** | Gherkin; mature; JUnit Platform and Maven; HTML report | Needs Java and Maven (optional, off by default) |

| Question | Options | Choice and reason |
|---|---|---|
| Step wording | Technical (`the output is "2"`); **language of the assignment** | `the program counts 2 valid account numbers` reads as a specification in the report; a technical step remains for exact output |
| Input in scenarios | Raw line only; **a column per value (b e m p u) where the input has five fields** | Every row reads as the input; malformed input stays one column |
| Java version | 21 (previous long-term support); **25 (current long-term support)** | The newest LTS release, also the default on the development machine; CI uses the same version (Temurin 25), so a difference between local and CI cannot come from Java |
| Integration | Separate command; **CTest test with label `e2e`, started by Maven** | `ctest -L e2e` runs it like every other test; CMake passes the path of the built program |

## Decision

A Maven project in `tests/e2e` (Java 25, Cucumber 8, JUnit 6, Jackson for the
HTML report), off by default (`IBAN_BUILD_E2E_TESTS`), with its own CI job and
report artifact. Scenario descriptions explain each rule for the examiner.

## Consequences

- 33 scenarios: the assignment examples, a single number valid or invalid, the
  input forms, the same count for p = 1 to 256, and every validation rule.
- The p table also checks the combination of the parallel count from end to
  end, which matters when the counting strategy changes.
- Major updates of Cucumber and JUnit must be done together (Cucumber 8 needs
  JUnit 6 and an explicit JSON library).