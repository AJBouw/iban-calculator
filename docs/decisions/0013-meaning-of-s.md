# 0013. Search mode: s is the account number to find

- **Status:** Accepted
- **Date:** 2026-11-14

## Context

The assignment describes the input field s and search mode (u = 2) as
follows:

> 6. Een optioneel getal s die alleen gebruikt wordt bij de zoekmodus.

> In de zoekmodus is het de bedoeling dat je een geldig rekeningnummer (komt
> uit het gegeven bereik en voldoet aan de m-proef) vindt. Wanneer een thread
> het gevonden heeft moet het programma zo snel mogelijk afsluiten. (De andere
> threads mogen niet veel verder zoeken) Als het rekeningnummer niet wordt
> gevonden dan moet het programma -1 printen.

From this text, the following is certain:

- s is a number, it is optional, and it is only used in search mode.
- A valid account number lies in `[b, e)` and passes the m-check.
- When a thread has found the number, the other threads must stop promptly.
- When the number is not found, the program prints `-1`.

The text does not say what s means: the description of search mode does not
mention s. The interpretation is therefore our own. The quotation leaves out a
stray "u" in two sentences; copying from the PDF loses characters (for
example, 2³¹ becomes "231"), so this is probably a copying artefact.

## Options

The examples use b = 274856170, e = 274856190, and m = 11; the valid numbers
in that range are 274856174 and 274856182.

| Option | s means | Example | For | Against |
|---|---|---|---|---|
| **A. The account number to find** | The program searches the range for s and prints s when it lies in `[b, e)` and passes the m-check | s = 274856182 prints 274856182; s = 274856183 prints -1 | Follows the text most literally: "het rekeningnummer" is one specific number, and the two conditions in brackets are exactly the checks for s; s is used, and only in search mode | The answer can also be computed directly, without threads; the threads search a range of which only one number matters |
| B. Not used | Find any valid number | Prints 274856174 or 274856182, depending on which thread is first | Fits "een geldig rekeningnummer"; the purest demonstration of stopping | s would be used nowhere, although the text says it is used in search mode; the answer differs per run; a valid number lies about m numbers further, so the search ends almost at once |
| C. A starting point | The smallest valid number ≥ s | s = 274856175 prints 274856182 | Uses s; the same answer in every run; an atomic minimum with compare-exchange | Not in the text; ends within about m numbers, so there is little to stop; threads below the best number found must continue, which makes stopping subtler |
| D. A fingerprint | The valid number whose hash equals s | s = hash(274856182) prints 274856182 | Real search work, so both the speedup and the early stop are measurable | The text mentions no hash; the hash function would be our own invention, and every test needs a precomputed s |
| E. A sequence number | The s-th valid number in the range | s = 2 prints 274856182 | Uses s; one answer | A thread cannot know how many valid numbers lie in the parts before it, so it cannot stop early; this conflicts with "de andere threads mogen niet veel verder zoeken" |
| F. Part of the number | A valid number that ends with the digits of s | s = 82 prints 274856182 | Real search work | Not in the text; several numbers can match |
| G. A weighted digit sum | A valid number whose weighted digit sum equals s | s = 220 prints 274856174 or 274856182 | Uses the m-check's own arithmetic | Not in the text; several numbers can match |

## Decision

Option **A**: s is the account number to find.

- In search mode, the p threads each search their own part of `[b, e)` for s.
  The number is found when it is s and passes the m-check; the program then
  prints s.
- When s lies outside `[b, e)` or does not pass the m-check, the program
  prints `-1`.
- s stays required in search mode; in count mode and list mode it may be
  omitted or given (as the `0` in the assignment examples), and it is ignored.
  The input parser already works this way, so it does not change.
- s is not checked against the range: a number outside `[b, e)` is a valid
  question with the answer `-1`.

How the other threads stop is a separate decision (0014).

## Consequences

- The weakness of A is stated openly: the answer could be computed directly
  with `passesMCheck(s, m)`. The threads search anyway, because the assignment
  is about the threads that find the number stopping the others. The
  measurement compares searching with and without stopping, so it shows what
  stopping saves.
- Only the thread whose part contains s can find it. The search treats "is it
  s?" as a condition it knows nothing about, so it does not use where s lies:
  when s is not valid, every thread searches its whole part before the answer
  is `-1`. Using the position of s is the direct computation above, which the
  algorithm step analyses separately.
- The threads only check numbers in `[b, e)`, so a negative s is never passed
  to the m-check, which throws for negative numbers.
- The interpretation should be confirmed with the lecturer. If another meaning
  is chosen later, a new record replaces this one; the threads, the stopping,
  and the tests for stopping stay the same, and only the condition for "found"
  changes.