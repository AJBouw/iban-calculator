# 0007. Threads: std::thread with explicit joins, one result slot per thread

- **Status:** Accepted; the result slots are superseded by [0010](0010-shared-counter-and-lock.md)
- **Date:** decided in the parallel count step (PR #14); recorded 2026-11-13

## Context

Count mode runs p threads, one per part. Every thread must be joined, also
when starting one of the threads fails, and the results must be combined
without a data race.

## Options

| Question | Options | Choice and reason |
|---|---|---|
| Thread type | **`std::thread` with explicit `join`**; `std::jthread` (C++20); a thread pool | `std::thread` as in the lectures, so every step is visible. `std::jthread` joins in its destructor and offers a `std::stop_token`; its advantages are explained in comments. A pool is introduced later, when threads are reused |
| Failure while starting threads | Ignore; **join the started threads, then rethrow** | A `std::thread` that is destroyed while joinable calls `std::terminate`; the running threads also use data on the caller's stack |
| Combining results | Shared counter with a lock; **one result slot per thread, added after joining** | Each thread writes only its own element, so no data is shared while the threads run and no lock is needed |

## Decision

`countParallel` starts one `std::thread` per part, joins all threads (also in
the error path) and adds the slots after joining. After `join`, every write of a
thread is visible to the caller.

## Consequences

- Correct and fast without locks: 7.8× speedup on 8 performance cores.
- The assignment asks for a shared counter in count mode; that is decided in
  [0010](0010-shared-counter-and-lock.md). The slot version remains as a
  baseline in the benchmark.