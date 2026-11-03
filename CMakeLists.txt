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
)

target_include_directories(poker_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if(BUILD_TESTING)
    add_executable(card_test
        tests/card_test.cc
    )

    target_link_libraries(card_test
        PRIVATE poker_core
    )

    add_test(
        NAME card_test
        COMMAND card_test
    )
endif()