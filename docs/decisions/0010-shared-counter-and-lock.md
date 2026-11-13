# 0010. Shared counter: local count, one locked add per thread, lock as a template parameter

- **Status:** Accepted; replaces the result slots of [0007](0007-threads-and-result-slots.md)
- **Date:** 2026-11-13

## Context

The assignment asks count mode to keep the result in a **shared counter**, and
the assessment criteria ask to protect shared mutable state against data races
and to hold locks for as little time as possible. The version with one result
slot per thread ([0007](0007-threads-and-result-slots.md)) needs no lock, but it
is not a shared counter. Three questions follow: how to protect the counter, how
often the threads touch it, and how the lock type reaches the counter.

## Options

### 1. Protection

| Option | For | Against |
|---|---|---|
| No protection | Fastest to write | Data race: undefined behaviour, lost updates |
| **`std::mutex` with `std::lock_guard`** | From the lectures; a waiting thread sleeps; RAII unlocks also on exceptions | A system call when many threads wait at once |
| `SpinLock` on `std::atomic_flag` | Cheap for a very short critical section with little waiting | Burns CPU while waiting; bad with more threads than cores (here p may be 256 on 10 cores) |
| `std::atomic<std::int64_t>` with `fetch_add` | No lock at all (lock-free) | Not a lock as in the lectures; with many threads the counter's cache line moves between cores on every add |

### 2. How often the shared counter is touched

| Option | Number of locked adds | Note |
|---|---|---|
| Once per valid number | About 9% of all numbers with m = 11, every number with m = 1 | Every thread waits for the same lock (contention) |
| **Count locally, add once per thread** | p in total | The lock is held p times for one addition |
| In batches (for example every 1000 numbers) | Between the two | Only useful for showing intermediate counts, which the assignment does not ask |

### 3. How the lock type reaches the counter

| Option | For | Against |
|---|---|---|
| **A. Template parameter, constrained by a C++20 concept** | Known at compile time, so `lock()` and `unlock()` can be inlined; templates are part of the course; the concept gives a clear error for a type without `lock()`/`unlock()` | The lock is chosen when compiling, not at run time |
| B. Virtual interface (a lock port) | Lock chosen at run time; fits ports and adapters | Every `lock()`/`unlock()` is a virtual call that is usually not inlined; with a lock per number the measurement would partly measure the interface |
| C. No abstraction, `std::mutex` written directly | Simplest | Every variant (mutex, spin lock) duplicates the counting code |
| D. Type erasure (for example `std::function`) | Run-time choice without inheritance | Costs about as much as B and is harder to explain |
| E. Template without a concept (C++17 style) | Same code as A | A wrong type gives long, unclear compiler errors |

### Also considered

- **Demonstrating the race.** A unit test that expects a wrong result is flaky:
  sometimes the result is right by chance, and an optimising compiler may
  combine the increments, which hides lost updates but not the data race. The
  race is therefore shown with ThreadSanitizer (`tsan` preset) and a separate
  demo program; the unit tests only check the protected variants.
- **False sharing.** The counter and its lock share a cache line on purpose:
  they are always used together. Nothing else that threads write lives next to
  them, so `alignas(std::hardware_destructive_interference_size)` is not needed.
- **Visibility.** The final value is read after all threads have been joined;
  `join` makes every write of a thread visible. The atomic variant can therefore
  use `std::memory_order_relaxed`.

## Decision

- The program counts with `SharedCounter<std::mutex>`: every thread counts its
  own part locally with `countValid` and adds the result **once** under the lock.
- The lock is a **template parameter constrained by the concept `BasicLockable`**
  (option A). `SpinLock` satisfies the same concept.
- Starting and joining the threads moves into one helper (`runInParallel`), so
  every variant uses the same, exception-safe thread handling.
- The other variants stay available for measurement and tests: result slots
  (the previous version), a lock per valid number with `std::mutex` and with
  `SpinLock`, and an atomic per valid number.

## Consequences

- The counter is shared and protected, as the assignment requires, and the lock
  is held only p times, as the assessment criteria ask.
- The speed of the program equals the result-slot version. Measured on the M1
  Pro (`docs/measurements.md`, step 5): 7.7 to 8.5 ms with 8 or 16 threads
  for ten million numbers, for both variants. With 16 threads, a lock per
  valid number takes 155 to 244 ms, and an atomic per valid number up to 806 ms.
- Changing the lock means recompiling; a run-time choice is not needed, because
  the program always uses one lock.
- The variants with a lock per number exist only to measure the cost of
  contention and are not used by the program.