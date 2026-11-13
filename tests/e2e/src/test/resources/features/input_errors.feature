# Invalid input as a user sees it: a message on standard error, nothing on
# standard output and exit code 1.
# The messages are checked partly ("contains"), so the wording around them may change.
Feature: Invalid input
  Invalid input is reported on standard error with the prefix "iban: error:".
  Nothing is printed on standard output and the exit code is 1, so a script
  can detect the failure without reading the message.

  Scenario: No input at all
    Given no input
    When the program runs
    Then the program fails
    And the output is empty
    And the error message contains "no input"

  Rule: The input must consist of 5 to 7 whole numbers
    With all values on one line, a missing value only shows as too few
    numbers: the program cannot tell which value is missing. These rows are a
    whole line in one column, because they do not have five valid fields.

    # LEARN: Several Examples blocks with a title group the rows in the report.
    Scenario Outline: <case>
      Given the input "<input>"
      When the program runs
      Then the program fails
      And the output is empty
      And the error message contains "<message>"

      Examples: Missing values
        | case                  | input                    | message                                        |
        | only b, e and m       | 274856170 274856190 11   | expected 5 to 7 numbers (b e m p u [s]), got 3 |
        | b, e, m and p, no u   | 274856170 274856190 11 4 | expected 5 to 7 numbers (b e m p u [s]), got 4 |

      Examples: Not a valid number or form
        | case                  | input                                 | message                                        |
        | text instead of b     | abc 274856190 11 4 0                  | not an integer: 'abc'                          |
        | too large for 64 bits | 99999999999999999999 274856190 11 4 0 | number too large                               |
        | too many numbers      | 0 274856170 274856190 11 4 0 0 9      | expected 5 to 7 numbers (b e m p u [s]), got 8 |
        | 7 numbers without 0   | 5 274856170 274856190 11 4 0 0        | with 7 numbers the first must be the leading 0 |

    # One answer per line: the input ends after b and e, so m, p and u are missing.
    Scenario: The input ends after e
      Given the input lines:
        """
        274856170
        274856190
        """
      When the program runs
      Then the program fails
      And the output is empty
      And the error message contains "expected 5 to 7 numbers (b e m p u [s]), got 2"

  Rule: Every value is checked against the rules of the assignment
    The values are checked in input order, so the message names the first
    invalid value. In every row only one value is invalid; the others are the
    values of the assignment example.

    # LEARN: The column names are the letters of the assignment, so a row reads as the input.
    Scenario Outline: <case>
      Given the input "<b> <e> <m> <p> <u>"
      When the program runs
      Then the program fails
      And the output is empty
      And the error message contains "<message>"

      Examples: Range of b and e
        | case           | b          | e          | m  | p | u | message                                         |
        | b below 0      | -1         | 274856190  | 11 | 4 | 0 | b must be between 0 and 2147483647              |
        | e below b      | 274856190  | 274856170  | 11 | 4 | 0 | e must be greater than b and at most 2147483647 |
        | e equal to b   | 274856170  | 274856170  | 11 | 4 | 0 | e must be greater than b and at most 2147483647 |
        | e above 2^31-1 | 2147483647 | 2147483648 | 11 | 4 | 0 | e must be greater than b and at most 2147483647 |

      Examples: Modulus m
        | case        | b         | e         | m   | p | u | message                     |
        | m below 1   | 274856170 | 274856190 | 0   | 4 | 0 | m must be between 1 and 256 |
        | m above 256 | 274856170 | 274856190 | 257 | 4 | 0 | m must be between 1 and 256 |

      Examples: Number of threads p
        | case        | b         | e         | m  | p   | u | message                     |
        | p below 1   | 274856170 | 274856190 | 11 | 0   | 0 | p must be between 1 and 256 |
        | p above 256 | 274856170 | 274856190 | 11 | 257 | 0 | p must be between 1 and 256 |

      Examples: Mode u
        | case                  | b         | e         | m  | p | u | message                                     |
        | unknown u             | 274856170 | 274856190 | 11 | 4 | 3 | u must be 0 (count), 1 (list) or 2 (search) |
        | search mode without s | 274856170 | 274856190 | 11 | 4 | 2 | search mode (u = 2) requires s              |
