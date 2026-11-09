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
    src/player_state.cc
    src/hand_state.cc
    src/hand_engine.cc
    src/random_agent.cc
    src/heuristic_agent.cc
    src/game_engine.cc
    src/match_runner.cc
    src/preflop_equity_table.cc
    src/self_play_runner.cc
    src/abstract_state.cc
    src/abstract_action.cc
    apps/poker_equity.cc
    apps/poker_hand_viewer.cc
    apps/benchmark_main.cc
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

    # Equity Calculator vs Unknown Opponent tests
    add_executable(equity_unknown_test tests/equity_unknown_test.cc)
    target_link_libraries(equity_unknown_test PRIVATE poker_core)
    add_test(NAME equity_unknown_test COMMAND equity_unknown_test)

    # CLI Poker Equity  
    add_executable(poker_equity apps/poker_equity.cc)
    target_link_libraries(poker_equity PRIVATE poker_core)

    # CLI Poker Equity Monte Carlo tests 
    add_test(NAME equity_cli_test COMMAND ${CMAKE_COMMAND} "-DCLI_EXE=$<TARGET_FILE:poker_equity>" -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/equity_cli_test.cmake")

    # Player State tests
    add_executable(player_state_test tests/player_state_test.cc)
    target_link_libraries(player_state_test PRIVATE poker_core)
    add_test(NAME player_state_test COMMAND player_state_test)

    # Hand State tests 
    add_executable(hand_state_test tests/hand_state_test.cc)
    target_link_libraries(hand_state_test PRIVATE poker_core)
    add_test(NAME HandStateTests COMMAND hand_state_test)

    # Hand Engine tests 
    add_executable(hand_engine_test tests/hand_engine_test.cc)
    target_link_libraries(hand_engine_test PRIVATE poker_core)
    add_test(NAME HandEngineTests COMMAND hand_engine_test)

    # Hand Engine stress tests 
    add_executable(hand_engine_stress_test tests/hand_engine_stress_test.cc)
    target_link_libraries(hand_engine_stress_test PRIVATE poker_core)
    add_test(NAME HandEngineStressTests COMMAND hand_engine_stress_test)
    set_tests_properties(HandEngineStressTests PROPERTIES LABELS "slow")

    # Player Observation tests
    add_executable(player_observation_test tests/player_observation_test.cc)
    target_link_libraries(player_observation_test PRIVATE poker_core)
    add_test(NAME player_observation_test COMMAND player_observation_test)

    # Legal Actions tests
    add_executable(legal_actions_test tests/legal_actions_test.cc)
    target_link_libraries(legal_actions_test PRIVATE poker_core)
    add_test(NAME legal_actions_test COMMAND legal_actions_test)

    # Random Agent tests
    add_executable(random_agent_test tests/random_agent_test.cc)
    target_link_libraries(random_agent_test PRIVATE poker_core)
    add_test(NAME random_agent_test COMMAND random_agent_test)

    # Heuristic Agent tests 
    add_executable(heuristic_agent_test tests/heuristic_agent_test.cc)
    target_link_libraries(heuristic_agent_test PRIVATE poker_core)
    add_test(NAME heuristic_agent_test COMMAND heuristic_agent_test)

    # CLI Hand Viewer
    add_executable(poker_hand_viewer apps/poker_hand_viewer.cc)
    target_link_libraries(poker_hand_viewer PRIVATE poker_core)

    # CLI Benchmark
    add_executable(poker_benchmark apps/benchmark_main.cc)
    target_link_libraries(poker_benchmark PRIVATE poker_core)

    # Game Engine tests
    add_executable(game_engine_test tests/game_engine_test.cc)
    target_link_libraries(game_engine_test PRIVATE poker_core)
    add_test(NAME game_engine_test COMMAND game_engine_test)

    # Match Runner tests 
    add_executable(match_runner_test tests/match_runner_test.cc)
    target_link_libraries(match_runner_test PRIVATE poker_core)
    add_test(NAME match_runner_test COMMAND match_runner_test)

    # Offline preflop equity generator
    add_executable(generate_preflop_table apps/generate_preflop_table.cc)
    target_link_libraries(generate_preflop_table PRIVATE poker_core)

    # Self-play runner tests
    add_executable(self_play_runner_test tests/self_play_runner_test.cc)
    target_link_libraries(self_play_runner_test PRIVATE poker_core)
    add_test(NAME self_play_runner_test COMMAND self_play_runner_test)

    # Abstract state tests
    add_executable(abstract_state_test tests/abstract_state_test.cc)
    target_link_libraries(abstract_state_test PRIVATE poker_core)
    add_test(NAME abstract_state_test COMMAND abstract_state_test)
endif()

# Poker Arena GUI
option(POKER_BUILD_GUI "Build the graphical Poker Arena" OFF)

if(POKER_BUILD_GUI)
    include(FetchContent)

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.2
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(raylib)

    add_executable(poker_arena apps/poker_arena.cc apps/arena/arena_view.cc)
    target_link_libraries(poker_arena PRIVATE poker_core raylib)
    target_compile_definitions(poker_arena PRIVATE
        POKER_ARENA_ASSET_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/assets/arena\"
    )
endif()