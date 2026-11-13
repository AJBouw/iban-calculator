# 0004. m-check: arithmetic, 64-bit numbers, invalid arguments throw

- **Status:** Accepted
- **Date:** decided in the m-check step (PR #10); recorded 2026-11-13

## Context

The m-check multiplies every digit by its position from the right (1, 2, 3, …)
and checks whether the sum is divisible by m. It is called once per number,
billions of times in total, from many threads at once.

## Options

| Question | Options | Choice and reason |
|---|---|---|
| Digits | Convert to text; **arithmetic with `% 10` and `/ 10`** | Arithmetic: no memory allocation, O(d) for d digits |
| Number type | `int`; **`std::int64_t`** | Headroom for the bounds (up to 2³¹ − 1) and intermediate results; no overflow in sums or range sizes |
| Invalid arguments (m < 1, negative number) | Return `false`; **throw `std::invalid_argument`** | Such values can only come from a programming error; `false` would silently give a wrong count, and `x % 0` is undefined behaviour |
| Shared state | Caches or lookup tables; **none** | Only arguments and local variables, so concurrent calls are safe without locks |
| Speed | Optimise now; **simple reference first** | Faster variants (an incremental walker) are added later and tested against this reference |

## Decision

`weightedDigitSum` and `passesMCheck` in the domain layer, as in the table.
The number 0 has sum 0 and therefore passes every m-check; whether 0 counts as
an account number is an open question in the README.

## Consequences

- Thread-safe by construction; every thread can call the m-check freely.
- Unit tests cover known sums, the bounds, every single-digit error, and every
  swap of neighbouring digits with m = 11, and the exceptions.
- Validated input never reaches the exceptions; they guard against bugs.