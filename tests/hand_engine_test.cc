
#include "poker/hand_engine.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

using poker::Card;
using poker::HandConfig;
using poker::HandEngine;
using poker::HandState;
using poker::Street;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

// Compile-time tests
static_assert(!std::is_default_constructible_v<HandEngine>);

static_assert(std::is_same_v<
    decltype(std::declval<const HandEngine&>().state()),
    const HandState&
>);

static_assert(noexcept(std::declval<const HandEngine&>().state()));

// Test fixtures
constexpr HandConfig defaultConfig{50, 100};

constexpr std::array<std::int64_t, 2> defaultStacks{
    1000, 1000
};

int main() {
    // Test 1: valid construction with button 0
    std::mt19937 rng0{42};

    HandEngine engine0{
        defaultStacks, defaultConfig, 0, rng0
    };

    check(engine0.state().button() == 0,
          "Button must be player 0");

    check(engine0.state().street() == Street::Preflop,
          "Initial street must be Preflop");

    check(engine0.state().board().empty(),
          "Initial board must be empty");

    check(engine0.state().pot() == 0,
          "Initial pot must be zero before posting blinds");

    check(!engine0.state().actingPlayer().has_value(),
          "No player must act before betting initialization");

    // Test 2: valid construction with button 1
    std::mt19937 rng1{42};

    HandEngine engine1{
        defaultStacks, defaultConfig, 1, rng1
    };

    check(engine1.state().button() == 1,
          "Button must be player 1");

    check(engine1.state().street() == Street::Preflop,
          "Initial street must be Preflop with button 1");

    check(engine1.state().board().empty(),
          "Board must be empty with button 1");

    // Test 3: initial stack conservation
    check(engine0.state().player(0).stack() == 1000,
          "Player 0 must retain initial stack");

    check(engine0.state().player(1).stack() == 1000,
          "Player 1 must retain initial stack");

    check(engine0.state().player(0).committedTotal() == 0,
          "Player 0 must have zero committed chips");

    check(engine0.state().player(1).committedTotal() == 0,
          "Player 1 must have zero committed chips");

    check(engine0.state().player(0).stack() +
          engine0.state().player(1).stack() +
          engine0.state().pot() == 2000,
          "Total chips must be conserved");

    // Test 4: all four hole cards must be distinct
    const auto& cards0 = engine0.state().player(0).holeCards();
    const auto& cards1 = engine0.state().player(1).holeCards();

    check(cards0[0] != cards0[1],
          "Player 0 hole cards must be distinct");

    check(cards1[0] != cards1[1],
          "Player 1 hole cards must be distinct");

    for (const Card& c0 : cards0) {
        for (const Card& c1 : cards1) {
            check(c0 != c1,
                  "Hole cards across players must be distinct");
        }
    }

    // Test 5: same seed and inputs produce identical cards
    std::mt19937 reproducibleRng{42};

    HandEngine sameEngine{
        defaultStacks, defaultConfig, 0, reproducibleRng
    };

    check(engine0.state().player(0).holeCards() ==
          sameEngine.state().player(0).holeCards(),
          "Same seed must reproduce player 0 hole cards");

    check(engine0.state().player(1).holeCards() ==
          sameEngine.state().player(1).holeCards(),
          "Same seed must reproduce player 1 hole cards");

    // Test 6: changing button swaps dealt card positions
    check(engine0.state().player(0).holeCards() ==
          engine1.state().player(1).holeCards(),
          "Button change must transfer first dealt hand to player 1");

    check(engine0.state().player(1).holeCards() ==
          engine1.state().player(0).holeCards(),
          "Button change must transfer second dealt hand to player 0");

    // Test 7: RNG advances after a valid construction
    std::mt19937 sequenceRng{12345};

    HandEngine firstEngine{
        defaultStacks, defaultConfig, 0, sequenceRng
    };

    std::mt19937 expectedRng{12345};

    HandEngine expectedFirst{
        defaultStacks, defaultConfig, 0, expectedRng
    };

    check(sequenceRng == expectedRng,
          "Identical successful constructions must advance RNG equally");

    HandEngine secondEngine{
        defaultStacks, defaultConfig, 0, sequenceRng
    };

    HandEngine expectedSecond{
        defaultStacks, defaultConfig, 0, expectedRng
    };

    check(secondEngine.state().player(0).holeCards() ==
          expectedSecond.state().player(0).holeCards(),
          "Sequential RNG use must remain reproducible");

    check(sequenceRng == expectedRng,
          "Sequential constructions must preserve RNG reproducibility");

    // Test 8: invalid blind configurations
    bool exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            defaultStacks, HandConfig{0, 100}, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Zero small blind must throw");

    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            defaultStacks, HandConfig{-10, 100}, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Negative small blind must throw");

    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            defaultStacks, HandConfig{100, 100}, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Equal blinds must throw");

    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            defaultStacks, HandConfig{100, 50}, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Small blind greater than big blind must throw");

    // Test 9: invalid button indices
    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            defaultStacks, defaultConfig, 2, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Button index 2 must throw");

    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            defaultStacks, defaultConfig, 255, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Button index 255 must throw");

    // Test 10: zero initial stacks
    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            {0, 1000}, defaultConfig, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Player 0 with zero initial stack must throw");

    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            {1000, 0}, defaultConfig, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Player 1 with zero initial stack must throw");

    // Test 11: negative initial stacks
    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            {-100, 1000}, defaultConfig, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Negative player 0 stack must throw");

    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            {1000, -100}, defaultConfig, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Negative player 1 stack must throw");

    // Test 12: total stack overflow
    const auto maxChips =
        std::numeric_limits<std::int64_t>::max();

    exceptionThrown = false;

    try {
        std::mt19937 rng{42};
        HandEngine invalid{
            {maxChips, 1}, defaultConfig, 0, rng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Total stacks exceeding INT64_MAX must throw");

    // Test 13: exact maximum chip capacity
    std::mt19937 maxRng{42};

    HandEngine maxEngine{
        {maxChips - 1, 1}, defaultConfig, 0, maxRng
    };

    check(maxEngine.state().player(0).stack() == maxChips - 1,
          "Maximum valid player 0 stack must be preserved");

    check(maxEngine.state().player(1).stack() == 1,
          "Maximum valid player 1 stack must be preserved");

    check(maxEngine.state().player(0).stack() +
          maxEngine.state().player(1).stack() == maxChips,
          "Exact INT64_MAX chip capacity must be accepted");

    // Test 14: invalid inputs must not consume RNG
    std::mt19937 invalidRng{9876};
    std::mt19937 untouchedRng{9876};

    exceptionThrown = false;

    try {
        HandEngine invalid{
            defaultStacks, HandConfig{100, 50}, 0, invalidRng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Invalid configuration must throw");

    check(invalidRng == untouchedRng,
          "Invalid configuration must not advance RNG");

    exceptionThrown = false;

    try {
        HandEngine invalid{
            defaultStacks, defaultConfig, 2, invalidRng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Invalid button must throw before shuffling");

    check(invalidRng == untouchedRng,
          "Invalid button must not advance RNG");

    exceptionThrown = false;

    try {
        HandEngine invalid{
            {0, 1000}, defaultConfig, 0, invalidRng
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Invalid stacks must throw before shuffling");

    check(invalidRng == untouchedRng,
          "Invalid stacks must not advance RNG");

    std::cout << "All HandEngine tests passed!\n";

    return EXIT_SUCCESS;
}
