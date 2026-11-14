# List mode (u = 1) as a user sees it: one line per valid account number, as
# "<sequence number> <account number>", and exit code 0.
# LEARN: The threads find the numbers in a different order every run, so the
# LEARN: scenarios compare the numbers as a set and check the sequence numbers apart.
Feature: List mode
  The input is a range of account numbers. In list mode (u = 1) the program
  writes every valid account number in the range as soon as a thread finds it,
  on its own line: the sequence number, a space and the account number. The
  order of the account numbers may differ per run, but the sequence numbers
  ascend from 1 in the order of the lines.

  # The examples from the assignment, with the input exactly as written there.
  @assignment
  Scenario: Example from the assignment, 11-check
    b = 274856170, e = 274856190, m = 11, p = 4, u = 1 (list). The assignment
    shows "1 274856182" and "2 274856174"; the order may also be the other way round.
    Given the input "0 274856170 274856190 11 4 1 0"
    When the program runs
    Then the program lists these valid account numbers:
      | 274856174 |
      | 274856182 |
    And the sequence numbers ascend from 1
    And the program succeeds

  @assignment
  Scenario: Example from the assignment, 12-check
    b = 374856170, e = 374856250, m = 12, p = 4, u = 1 (list). The same six
    numbers that count mode counts for this range.
    Given the input "0 374856170 374856250 12 4 1 0"
    When the program runs
    Then the program lists these valid account numbers:
      | 374856173 |
      | 374856181 |
      | 374856202 |
      | 374856210 |
      | 374856238 |
      | 374856246 |
    And the sequence numbers ascend from 1
    And the program succeeds

  Rule: A range without valid account numbers gives no lines
    The range [n, n + 1) holds only n; 274856183 does not pass the 11-check.

    Scenario: No valid account number in the range
      Given the input "274856183 274856184 11 4 1"
      When the program runs
      Then the program lists no account numbers
      And the program succeeds

  Rule: The number of threads does not change the list
    Every thread writes the numbers of its own part. A part written twice or
    not at all, or two threads writing at the same time, would change the lines.

    # LEARN: The column names are the letters of the assignment, so a row reads as the input.
    Scenario Outline: Listing with p = <p>
      Given the input "<b> <e> <m> <p> <u>"
      When the program runs
      Then the program lists these valid account numbers:
        | 274856174 |
        | 274856182 |
      And the sequence numbers ascend from 1
      And the program succeeds

      Examples:
        | b         | e         | m  | p   | u | why                                 |
        | 274856170 | 274856190 | 11 | 1   | 1 | sequential, the reference           |
        | 274856170 | 274856190 | 11 | 2   | 1 | one valid number in each part       |
        | 274856170 | 274856190 | 11 | 3   | 1 | parts of unequal size (7, 7, 6)     |
        | 274856170 | 274856190 | 11 | 20  | 1 | one number per thread               |
        | 274856170 | 274856190 | 11 | 256 | 1 | more threads than numbers, the most |
