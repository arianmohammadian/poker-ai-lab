cmake_minimum_required(VERSION 3.16)

project(PokerAILab
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(poker_core
    src/card.cc
    src/deck.cc
    src/hand_rank.cc
    src/hand_evaluator.cc
    src/equity_calculator.cc
    apps/poker_equity.cc
)

target_include_directories(poker_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if(BUILD_TESTING)
    # Card tests
    add_executable(card_test tests/card_test.cc)
    target_link_libraries(card_test PRIVATE poker_core)
    add_test(NAME card_test COMMAND card_test)

    # Deck tests 
    add_executable(deck_test tests/deck_test.cc)
    target_link_libraries(deck_test PRIVATE poker_core)
    add_test(NAME deck_test COMMAND deck_test)

    # Hand Rank tests
    add_executable(hand_rank_test tests/hand_rank_test.cc)
    target_link_libraries(hand_rank_test PRIVATE poker_core)
    add_test(NAME hand_rank_test COMMAND hand_rank_test)

    # HandEvaluator tests (5 cards)
    add_executable(hand_evaluator_test tests/hand_evaluator_test.cc)
    target_link_libraries(hand_evaluator_test PRIVATE poker_core)
    add_test(NAME hand_evaluator_test COMMAND hand_evaluator_test)

    # HandEvaluator tests (7 cards)
    add_executable(seven_card_hand_evaluator_test tests/seven_card_hand_evaluator_test.cc)
    target_link_libraries(seven_card_hand_evaluator_test PRIVATE poker_core)
    add_test(NAME seven_card_hand_evaluator_test COMMAND seven_card_hand_evaluator_test)

    # Equity Result tests
    add_executable(equity_result_test tests/equity_result_test.cc)
    target_link_libraries(equity_result_test PRIVATE poker_core)
    add_test(NAME equity_result_test COMMAND equity_result_test)

    # Equity Input tests 
    add_executable(equity_input_test tests/equity_input_test.cc)
    target_link_libraries(equity_input_test PRIVATE poker_core)
    add_test(NAME equity_input_test COMMAND equity_input_test)

    # Equity Calculator tests (flop, turn and river)
    add_executable(equity_calculator_test tests/equity_calculator_test.cc)
    target_link_libraries(equity_calculator_test PRIVATE poker_core)
    add_test(NAME equity_calculator_test COMMAND equity_calculator_test)

    # Equity Calculator tests (preflop)
    add_executable(equity_preflop_test tests/equity_preflop_test.cc)
    target_link_libraries(equity_preflop_test PRIVATE poker_core)
    add_test(NAME equity_preflop_test COMMAND equity_preflop_test)
    set_tests_properties(equity_preflop_test PROPERTIES LABELS "slow")

    # Equity Calculator Monte Carlo tests 
    add_executable(equity_monte_carlo_test tests/equity_monte_carlo_test.cc)
    target_link_libraries(equity_monte_carlo_test PRIVATE poker_core)
    add_test(NAME equity_monte_carlo_test COMMAND equity_monte_carlo_test)

    # CLI Poker Equity tests 
    add_executable(poker_equity apps/poker_equity.cc)
    target_link_libraries(poker_equity PRIVATE poker_core)

    # CLI Poker Equity Monte Carlo tests 
    add_test(
        NAME equity_cli_test
        COMMAND ${CMAKE_COMMAND}
            "-DCLI_EXE=$<TARGET_FILE:poker_equity>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/equity_cli_test.cmake"
    )
endif()