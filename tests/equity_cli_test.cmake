# Integration tests for the interactive poker_equity executable.
# Invoke with: cmake -DCLI_EXE=/path/to/poker_equity -P tests/equity_cli_test.cmake
# No test framework or changes to production code required.

if(NOT DEFINED CLI_EXE OR NOT EXISTS "${CLI_EXE}")
    message(FATAL_ERROR "CLI_EXE must point to the built poker_equity executable")
endif()

function(run_case name input)
    set(input_file "${CMAKE_CURRENT_BINARY_DIR}/poker_cli_${name}.txt")
    file(WRITE "${input_file}" "${input}")
    execute_process(
        COMMAND "${CLI_EXE}"
        INPUT_FILE "${input_file}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE status
        TIMEOUT 60
    )
    file(REMOVE "${input_file}")

    if(NOT "${status}" STREQUAL "0")
        message(FATAL_ERROR "${name}: process failed (${status})\nstdout:\n${output}\nstderr:\n${error}")
    endif()

    set(case_stdout "${output}" PARENT_SCOPE)
    set(case_stderr "${error}" PARENT_SCOPE)
    message(STATUS "Executed: ${name}")
endfunction()

function(expect_contains name value needle)
    string(FIND "${value}" "${needle}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "${name}: expected text not found: [${needle}]\nActual:\n${value}")
    endif()
endfunction()

function(expect_empty name value)
    if(NOT "${value}" STREQUAL "")
        message(FATAL_ERROR "${name}: expected empty stderr, got:\n${value}")
    endif()
endfunction()

# River: the two methods must agree, and sample count/seed must be printed.
run_case(exact_river "Ah As\nKh Ks\n2c 7d Js 4c 9h\n1\nn\n")
expect_contains(exact_river "${case_stdout}" "Player 1 equity: 100.00%")
expect_contains(exact_river "${case_stdout}" "Player 2 equity: 0.00%")
expect_contains(exact_river "${case_stdout}" "Scenarios:     1")
expect_contains(exact_river "${case_stdout}" "Method:        Exact")
expect_empty(exact_river "${case_stderr}")

run_case(mc_river "Ah As\nKh Ks\n2c 7d Js 4c 9h\n2\n7\n0\nn\n")
expect_contains(mc_river "${case_stdout}" "Player 1 equity: 100.00%")
expect_contains(mc_river "${case_stdout}" "Player 1 wins: 7")
expect_contains(mc_river "${case_stdout}" "Scenarios:     7")
expect_contains(mc_river "${case_stdout}" "Method:        Monte Carlo")
expect_contains(mc_river "${case_stdout}" "Seed:          0")
expect_empty(mc_river "${case_stderr}")

# Repeating an identical seeded Monte Carlo request must yield identical output.
set(mc_request "Ah As\nKh Ks\n2c 7d Js\n2\n250\n11\nn\n")
run_case(mc_repeat_1 "${mc_request}")
set(first_output "${case_stdout}")
expect_empty(mc_repeat_1 "${case_stderr}")
run_case(mc_repeat_2 "${mc_request}")
if(NOT "${case_stdout}" STREQUAL "${first_output}")
    message(FATAL_ERROR "Same input and seed produced different CLI output")
endif()
expect_contains(mc_repeat_2 "${case_stdout}" "Scenarios:     250")

# Input errors should be handled by the CLI, not terminate the process.
run_case(bad_rank "Xh As\nn\n")
expect_contains(bad_rank "${case_stderr}" "Invalid card rank")

run_case(extra_hand_card "Ah As Kh\nn\n")
expect_contains(extra_hand_card "${case_stderr}" "exactly two cards")

run_case(bad_board_count "Ah As\nKh Ks\n2c 7d\nn\n")
expect_contains(bad_board_count "${case_stderr}" "Board must contain 0, 3, 4 or 5 cards")

run_case(duplicate_cards "Ah As\nAh Ks\n\n1\nn\n")
expect_contains(duplicate_cards "${case_stderr}" "Duplicated cards")

run_case(bad_mode "Ah As\nKh Ks\n\n3\nn\n")
expect_contains(bad_mode "${case_stderr}" "Invalid calculation mode")

run_case(zero_samples "Ah As\nKh Ks\n\n2\n0\n42\nn\n")
expect_contains(zero_samples "${case_stderr}" "Sample count must be positive")

run_case(negative_samples "Ah As\nKh Ks\n\n2\n-1\nn\n")
expect_contains(negative_samples "${case_stderr}" "Invalid unsigned integer")

run_case(trailing_samples "Ah As\nKh Ks\n\n2\n12abc\nn\n")
expect_contains(trailing_samples "${case_stderr}" "Invalid unsigned integer")

run_case(overflow_samples "Ah As\nKh Ks\n\n2\n18446744073709551616\nn\n")
expect_contains(overflow_samples "${case_stderr}" "Integer is out of range")

message(STATUS "All Poker Equity CLI integration checks passed.")
