# 0006. Range split: exactly p contiguous parts

- **Status:** Accepted
- **Date:** decided in the range split step (PR #13); recorded 2026-11-13

## Context

The assignment requires that the range [b, e) is divided between exactly p
threads without gaps or overlap, and that the workloads differ by at most one
number. p may be larger than the number of values.

## Options

| Question | Options | Choice and reason |
|---|---|---|
| Representation | Closed ranges [b, e − 1]; **half-open `Range{begin, end}`** | Matches [b, e) in the assignment; neighbouring parts share a boundary without overlap; an empty range is `begin == end` |
| Pattern | **Contiguous blocks**; interleaving (x, x + p, x + 2p, …) | Blocks: each thread works on consecutive numbers, which an incremental m-check can reuse and which keeps memory access local. Interleaving balances automatically but prevents that reuse |
| Sizes | Last part takes the remainder; **first n mod p parts one larger** | Sizes differ by at most one, as required |
| More threads than numbers | Fewer threads; **exactly p parts, some empty** | The assignment asks for p threads; a thread with an empty part finishes at once |
| Layer | With the threads; **domain, pure arithmetic** | Testable without threads; O(p), independent of the size of the range |

## Decision

`splitRange(range, p)` returns exactly p half-open contiguous parts; with
n = e − b, every part gets n / p values, and the first n mod p parts get one more.

## Consequences

- Tests check the three requirements (no gaps, no overlap, sizes differ by at
  most one) for thousands of combinations, including the largest range.
- Measurements showed that equal parts are not optimal on a processor with
  performance and efficiency cores (see [0008](0008-measurements.md)); dynamic
  distribution is investigated with the thread pool.