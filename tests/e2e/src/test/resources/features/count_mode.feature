Feature: Count mode
  The program counts the account numbers in [b, e) that pass the m-check
  and prints one number.

  Scenario: Example from the assignment
    Given the input "0 274856170 274856190 11 4 0 0"
    When the program runs
    Then the output is "2"
    And the program succeeds