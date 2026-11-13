# 0005. Input: forms, questions per field, one source for the rules

- **Status:** Accepted
- **Date:** decided in the input step (PR #12); recorded 2026-11-13

## Context

The assignment describes five values and an optional sixth (b e m p u [s]),
but its examples contain seven numbers that start with an extra 0. A user may
type the values one by one or paste a line from the assignment. The rules for
every value (for example 1 ≤ p ≤ 256) are needed by the parser, the questions,
the error messages, the defaults and the tests.

## Options

| Question | Options | Choice and reason |
|---|---|---|
| Seven numbers | Reject; ignore the first number; **accept only a leading 0** | The examples work as written, and any other first value is rejected, so a mistyped line is not silently misread. To be confirmed with the lecturer |
| Way of entering | One line only; **one question per value, a pasted line also accepted** | Clear for a new user, and the assignment examples can still be pasted |
| Empty answer | End the input; **ask again** for required values | Prevents a missing value by accident; `s` may be skipped |
| Where prompts go | Standard output; **standard error** | Standard output holds only the result, so scripts and tests can read it |
| Invalid user input | Throw; **return a result with a message** | A typing error is expected, unlike a programming error (see [0004](0004-m-check.md)); `std::expected` would fit but needs C++23 |
| Where the rules live | In every place that needs them; **one header, `Limits.h`** | Parser, questions, messages and tests are built from the same constants and cannot disagree |
| Missing `s` | Use `-1` or `0`; **`std::optional`** | Every number could be real input, so absence needs its own representation |
| Mode | `int`; **`enum class Mode`** | Cannot be mixed up with a number; the compiler warns when a `switch` misses a mode |

## Decision

`ConsoleInput` asks the questions and joins the answers into one line;
`InputParser` converts the line with `std::from_chars` and checks the values in
input order, so the message names the first invalid one. Only the parser creates
a `Query`, so a `Query` is always valid. Errors are printed as `iban: error: …`
with exit code 1.

## Consequences

- The rest of the program does not repeat input checks.
- The end-to-end scenarios check every rule with its message, standard output
  and exit code.
- The interpretation of the leading 0 is an assumption, listed in the README.