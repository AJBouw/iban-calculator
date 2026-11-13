# Count mode (u = 0) as a user sees it: the values on standard input, one
# number on standard output and exit code 0.
# LEARN: A .feature file is plain text in Gherkin; IbanSteps.java links every step to code.
# LEARN: Text directly under Feature, Rule or Scenario is a description and appears in the report.
Feature: Count mode
  The input is a range of account numbers, not a single number: every number
  from b up to, but not including, e is checked with the m-check. In count
  mode (u = 0) the program prints how many of them are valid.

  # The examples from the assignment, with the input exactly as written there.
  # LEARN: Run only these with: mvn test -Dcucumber.filter.tags=@assignment
  @assignment
  Scenario: Example from the assignment
    b = 274856170, e = 274856190, m = 11, p = 4, u = 0 (count). The extra 0 at
    the start and s = 0 at the end are as written in the assignment.
    Of the 20 numbers, 274856174 and 274856182 pass the 11-check.
    Given the input "0 274856170 274856190 11 4 0 0"
    When the program runs
    Then the program counts 2 valid account numbers
    And the program succeeds

  @assignment
  Scenario: Range of the list example, counted
    b = 374856170, e = 374856250, m = 12, p = 4, u = 0 (count). The assignment
    lists six numbers for this range with m = 12; count mode must find the same six.
    Given the input "374856170 374856250 12 4 0"
    When the program runs
    Then the program counts 6 valid account numbers
    And the program succeeds

  Rule: A single account number is valid or not
    The range [n, n + 1) holds only n, so the count is 1 when n passes the
    m-check (valid) and 0 when it does not (invalid).

    Scenario Outline: <number> is <result>
      Given the input "<b> <e> <m> <p> <u>"
      When the program runs
      Then the program counts <count> valid account numbers
      And the program succeeds

      Examples:
        | number    | result  | b         | e         | m  | p | u | count |
        | 274856182 | valid   | 274856182 | 274856183 | 11 | 1 | 0 | 1     |
        | 274856183 | invalid | 274856183 | 274856184 | 11 | 1 | 0 | 0     |

  Rule: The result does not depend on how the values are entered
    The values can be pasted on one line in three forms, or typed one per
    question. Each form below gives b = 274856170, e = 274856190, m = 11,
    p = 4 and u = 0.

    # The whole line is one column here, because the forms differ in the number of fields.
    # LEARN: Scenario Outline is a template; every row under Examples becomes one scenario.
    # LEARN: <form> and <input> are replaced by the values of that row, also inside quotes.
    Scenario Outline: All values on one line: <form>
      Given the input "<input>"
      When the program runs
      Then the program counts 2 valid account numbers
      And the program succeeds

      Examples:
        | form                 | input                          |
        | 7 numbers, leading 0 | 0 274856170 274856190 11 4 0 0 |
        | 6 numbers, with s    | 274856170 274856190 11 4 0 0   |
        | 5 numbers, without s | 274856170 274856190 11 4 0     |

    # One answer per line in the order of the questions: b, e, m, p, u. s is
    # optional in count mode, so the input ends before the last question.
    # LEARN: A DocString (between """) passes several lines to one step.
    Scenario: One value per question
      Given the input lines:
        """
        274856170
        274856190
        11
        4
        0
        """
      When the program runs
      Then the program counts 2 valid account numbers
      And the program succeeds

    # The empty line is the answer to the question for e, which is then asked again.
    Scenario: An empty answer to a required question is asked again
      Given the input lines:
        """
        274856170

        274856190
        11
        4
        0
        """
      When the program runs
      Then the program counts 2 valid account numbers
      And the program succeeds

  Rule: The number of threads does not change the result
    Every thread counts its own part of the range and the parts are added at
    the end. A part counted twice or not at all would change the result.

    # The range [274856170, 274856190) holds 20 numbers; only p changes per row.
    # LEARN: The column names are the letters of the assignment, so a row reads as the input.
    Scenario Outline: Counting with p = <p>
      Given the input "<b> <e> <m> <p> <u>"
      When the program runs
      Then the program counts 2 valid account numbers
      And the program succeeds

      Examples:
        | b         | e         | m  | p   | u | why                                 |
        | 274856170 | 274856190 | 11 | 1   | 0 | sequential, the reference           |
        | 274856170 | 274856190 | 11 | 2   | 0 | two equal parts                     |
        | 274856170 | 274856190 | 11 | 3   | 0 | parts of unequal size (7, 7, 6)     |
        | 274856170 | 274856190 | 11 | 8   | 0 | one thread per performance core     |
        | 274856170 | 274856190 | 11 | 20  | 0 | one number per thread               |
        | 274856170 | 274856190 | 11 | 256 | 0 | more threads than numbers, the most |
