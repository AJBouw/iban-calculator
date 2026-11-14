# Search mode (u = 2) as a user sees it: the account number s, or -1, and exit
# code 0. The meaning of s is our own interpretation (decision 0013): s is the
# account number to find.
# LEARN: The assignment gives no example for search mode, so no scenario here
# LEARN: has the @assignment tag; the numbers come from the list mode example.
Feature: Search mode
  The input is a range of account numbers and an account number s. In search
  mode (u = 2) the threads search their parts of the range for s. The program
  prints s when it lies in the range and passes the m-check; the thread that
  finds it stops the other threads. Otherwise, the program prints -1.

  # 274856174 and 274856182 are the valid numbers in [274856170, 274856190)
  # for the 11-check, as in the list mode example of the assignment.
  Scenario: The account number is in the range and passes the m-check
    b = 274856170, e = 274856190, m = 11, p = 4, u = 2 (search), s = 274856182,
    written in the same form as the assignment examples.
    Given the input "0 274856170 274856190 11 4 2 274856182"
    When the program runs
    Then the program finds account number 274856182
    And the program succeeds

  Rule: Both ends of the range belong to the search, except e
    b is inclusive and e is exclusive. In [274856174, 274856183) the first and
    the last number are both valid.

    Scenario Outline: s at the <where> of the range
      Given the input "<b> <e> <m> <p> <u> <s>"
      When the program runs
      Then the program finds account number <s>
      And the program succeeds

      Examples:
        | b         | e         | m  | p | u | s         | where |
        | 274856174 | 274856183 | 11 | 4 | 2 | 274856174 | start |
        | 274856174 | 274856183 | 11 | 4 | 2 | 274856182 | end   |

  Rule: An account number that is not found gives -1
    The program succeeds: "not found" is an answer, not an error.

    Scenario Outline: s = <s>, <why>
      Given the input "<b> <e> <m> <p> <u> <s>"
      When the program runs
      Then the program finds no account number
      And the program succeeds

      Examples:
        | b         | e         | m  | p | u | s         | why                         |
        | 274856170 | 274856190 | 11 | 4 | 2 | 274856183 | it fails the 11-check       |
        | 274856170 | 274856190 | 11 | 4 | 2 | 274856169 | it lies below b             |
        | 274856170 | 274856190 | 11 | 4 | 2 | 274856190 | e itself is not in the range |

  Rule: The number of threads does not change the answer
    Only the thread whose part contains s can find it. With more threads, s
    lies in another part, but the answer stays the same.

    Scenario Outline: Searching with p = <p>
      Given the input "<b> <e> <m> <p> <u> <s>"
      When the program runs
      Then the program finds account number 274856182
      And the program succeeds

      Examples:
        | b         | e         | m  | p   | u | s         | why                                 |
        | 274856170 | 274856190 | 11 | 1   | 2 | 274856182 | sequential, the reference           |
        | 274856170 | 274856190 | 11 | 2   | 2 | 274856182 | s in the second part                |
        | 274856170 | 274856190 | 11 | 3   | 2 | 274856182 | parts of unequal size (7, 7, 6)     |
        | 274856170 | 274856190 | 11 | 20  | 2 | 274856182 | one number per thread               |
        | 274856170 | 274856190 | 11 | 256 | 2 | 274856182 | more threads than numbers, the most |
