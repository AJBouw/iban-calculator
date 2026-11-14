# 0014. Search mode: an atomic stop flag, checked before every number

- **Status:** Accepted
- **Date:** 2026-11-14

## Context

In search mode the p threads each search their own part of `[b, e)` for the
account number s ([0013](0013-meaning-of-s.md)). The assignment requires that
the program ends as soon as possible once a thread has found it: "De andere
threads mogen niet veel verder zoeken". A thread cannot be stopped from
outside in C++, so every thread has to ask itself, regularly, whether it should
stop. The decision is what the threads look at, how often they look, and how
the answer gets back to the main thread.

## Options

| Option | How | For | Against |
|---|---|---|---|
| **1. `std::atomic<bool>` stop flag** | The thread that finds s sets the flag; every thread reads it in its loop | Simple and visible; lock-free. Reading is cheap: as long as nobody writes the flag, every core keeps it in its own cache, so it does not move between cores like the atomic counter of [0010](0010-shared-counter-and-lock.md) | The memory order has to be chosen and explained |
| 2. `std::stop_source` and `std::stop_token` (C++20) | The standard way to ask a thread to stop; works with a plain `std::thread` | Standard, and clear in intent | Hides the atomic inside, so it shows less of what happens; `std::jthread` would also replace the explicit joins of [0007](0007-threads-and-result-slots.md) |
| 3. `bool` under a mutex | Lock, read the flag, unlock | Known from the lectures | A lock for every number checked; step 5 measured how expensive a lock per number is |
| 4. End the program at once | The thread that finds s prints it and calls `std::exit()` | Literally "zo snel mogelijk afsluiten" | The other threads keep running while the program cleans up (undefined behaviour), and it cannot be unit tested. Rejected |

### Also considered

- **How often to look.** Before every number, or before every 1024th number.
  Before every number, a thread stops after at most one more number; every
  1024 numbers makes the check even cheaper, but a thread may search up to
  1024 numbers further. Both satisfy "niet veel verder zoeken", so the
  interval is a template parameter and both are measured, with "never" as the
  baseline.
- **Stopping when s is not valid.** The thread whose part contains s could
  stop the others as soon as it has checked s, also when the answer is `-1`.
  That uses the position of s, which is the direct computation of
  [0013](0013-meaning-of-s.md); the search treats the condition as a black box
  instead, so it works unchanged for another meaning of s.
- **Returning the answer.** Every thread could write to its own result slot,
  as in step 4. An `std::atomic<std::int64_t>` with `compare_exchange_strong`
  keeps only the first answer and needs no slots. With this meaning of s at
  most one thread can find a number, but the search does not rely on that.
- **The memory order.** The flag only says "stop", and the answer is read after
  `join`. `join` already makes every write of a thread visible to the caller,
  so `memory_order_relaxed` is enough for both the flag and the answer;
  stronger orders would only add cost on processors such as ARM (the M1 Pro).
- **One flag per search.** The flag and the answer are local variables of one
  search, not globals, so a thread pool can run several searches after each
  other without resetting shared state.

## Decision

- Option **1**: `std::atomic<bool>` with relaxed loads and stores, in
  `searchWithStopFlag<CheckInterval>` (`SearchVariants.h`).
- The program uses an interval of **1**: the flag is checked before every
  number (`searchParallel`). The intervals 1024 and "never" exist for the
  measurement.
- The answer is an `std::atomic<std::int64_t>`, set with
  `compare_exchange_strong`; it starts as `-1`, the answer for "not found".
- `findValid` (`FindValid.h`) does the search on one thread and gets the stop
  check and the condition as parameters, so the domain layer knows nothing
  about threads or atomics.

## Consequences

- The other threads stop after at most one more number. The unit test
  `StopTheOtherThreadsWhenTheNumberIsFound` searches the largest range for
  s = 0: without stopping it would take seconds, with the flag it ends at
  once.
- When s is not found, every thread searches its whole part; the flag is then
  read for every number without being set, which is the cost the measurement
  shows.
- The search is ready for a thread pool: nothing has to be reset between two
  searches.
- The measurement is in `docs/measurements.md` (step 7).