# 0012. List mode: sequence number and line under one lock

- **Status:** Accepted
- **Date:** 2026-11-13

## Context

In list mode the assignment asks for one line per valid account number: the
sequence number, a space, and the account number. A number must be written as
soon as it is found ("je mag niet alles tot het eind opsparen"). The order of
the account numbers may differ per run, but the sequence numbers must ascend in
the order of the lines. Two things are shared by all threads: the next sequence
number and the output stream. The assessment criteria ask to hold locks as
briefly as possible.

## Options

| Option | How | For | Against |
|---|---|---|---|
| **A. Sequence number and line under one lock** | Lock, increase the sequence number, write the line, unlock | Correct by construction; follows the assignment literally; little code | The lock is held while writing (I/O); one lock per valid number, which [0010](0010-shared-counter-and-lock.md) showed to be slow under contention |
| B. Atomic sequence number, line written separately | `fetch_add`, then write under a lock (or without one) | No lock for the number | **Wrong**: another thread can take the next number and write first, so a higher number appears before a lower one. Without a lock for the stream, lines can also be mixed |
| C. Collect per thread, write after `join` | Every thread keeps its numbers; the main thread writes them | No shared state while the threads run | Not allowed: the numbers must be written at once |
| D. One writer thread with a bounded buffer | The counting threads push numbers into a bounded buffer; one extra thread numbers and writes them (producer-consumer) | The stream and the sequence number belong to one thread, so they need no lock; the lock is held only to push; the bounded buffer and condition variables are part of the lectures | More code and one extra thread; a number is written when the writer gets to it, not by the finding thread; every number is handed over between threads |

### Also considered

- **Formatting outside the lock.** In A, the account number is converted to
  text before the lock is taken; under the lock only the sequence number is
  increased and one string is written.
- **Flushing.** `std::endl` or a flush after every line would make large lists
  much slower when the output is piped. On a terminal the output appears line
  by line anyway, and a buffer of a few kilobytes is not "saving everything
  until the end". The lines are written with `'\n'`.
- **Testable output.** The variants write to a `std::ostream&`, so the tests
  write to a `std::ostringstream` and check every line.
- **The extra thread in D.** The assignment's p threads are the threads that
  compute; the writer in D only writes, so p counting threads remain.
- **B as a demonstration.** B is a race condition without a data race: every
  access is synchronised, so ThreadSanitizer reports nothing, yet the result is
  wrong. `tests/race/ListOrderDemo.cpp` shows it.

## Decision

- The program uses **A** with `std::mutex`: `SequencedWriter<Lock>` holds the
  stream, the lock and the sequence number, so no caller can write a line
  without the lock (`listWithLock`, used by `listParallel`).
- **D** is implemented as `listWithWriterThread` with `BoundedBuffer<T>` (a
  mutex and two condition variables), tested, and measured against A.
- **B** exists only in the demonstration program.
- The lock is a template parameter, as in [0010](0010-shared-counter-and-lock.md),
  so `SpinLock` is measured with the same code.

## Consequences

- The sequence numbers always ascend in the order of the lines; the unit tests
  and the end-to-end scenarios check this for many thread counts.
- With many valid numbers (small m) the threads wait for the output lock: list
  mode cannot scale like count mode, because writing is sequential by nature.
  More threads even make list mode slower (see *Evidence*).
- D is kept in the code as a measured alternative, not as the program. Batching
  the numbers before they are handed over is an option for the performance
  step.
- The bounded buffer is reused when the thread pool arrives (a queue of tasks).

## Evidence

Measured on the M1 Pro on 2026-11-14, range `[0, 10⁶)`, median of 5
repetitions (`docs/measurements.md`, step 6; raw data in
`docs/data/step6-list.json`):

| Variant | m = 11, p = 1 | m = 11, p = 16 | m = 1, p = 1 | m = 1, p = 16 |
|---|---:|---:|---:|---:|
| **A, `std::mutex`** (the program) | **10.9 ms** | **20.2 ms** | **69.4 ms** | **135 ms** |
| A, `SpinLock` | 10.1 ms | 17.9 ms | 61.3 ms | 92.0 ms |
| D, writer thread | 16.5 ms | 148 ms | 150 ms | 1634 ms |

- A with `std::mutex` is stable for every p (spread at most 4%). The spin lock
  is faster at some thread counts, but very unstable at p = 8 (spread 16%), as
  in [0010](0010-shared-counter-and-lock.md).
- D is slower than A for every p and m, up to 12× with m = 1 and 16 threads:
  every number costs two lock operations and a wake-up instead of one lock
  operation.
- `iban_list_order_demo` with 200,000 lines and 8 threads: B wrote 6,557 to
  7,618 lines with a lower sequence number than the line before, per run; A
  wrote none. ThreadSanitizer reported nothing for either.