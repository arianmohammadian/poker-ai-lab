
#include "poker/hand_state.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

using poker::Card;
using poker::HandState;
using poker::PlayerState;
using poker::Rank;
using poker::Street;
using poker::Suit;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

// Compile-time tests
static_assert(!std::is_default_constructible_v<HandState>);

static_assert(std::is_same_v<
    decltype(std::declval<const HandState&>().player(0)),
    const PlayerState&
>);

static_assert(std::is_same_v<
    decltype(std::declval<const HandState&>().board()),
    const std::vector<Card>&
>);

// Test fixtures
const std::array<Card, 2> holeCards0 = {
    Card{Rank::Ace, Suit::Spades},
    Card{Rank::King, Suit::Hearts}
};

const std::array<Card, 2> holeCards1 = {
    Card{Rank::Queen, Suit::Diamonds},
    Card{Rank::Jack, Suit::Clubs}
};

std::array<PlayerState, 2> makePlayers(
    std::int64_t stack0 = 1000,
    std::int64_t stack1 = 1000
) {
    return {
        PlayerState{stack0, holeCards0},
        PlayerState{stack1, holeCards1}
    };
}

int main() {
    // Test 1: valid construction with button 0
    HandState hand0{makePlayers(), 0};

    check(hand0.button() == 0,
          "Button must be player 0");

    check(hand0.street() == Street::Preflop,
          "Initial street must be Preflop");

    check(hand0.board().empty(),
          "Initial board must be empty");

    check(!hand0.actingPlayer().has_value(),
          "No player must be acting before betting starts");

    check(hand0.pot() == 0,
          "Initial pot must be zero");

    // Test 2: valid construction with button 1
    HandState hand1{makePlayers(), 1};

    check(hand1.button() == 1,
          "Button must be player 1");

    check(hand1.street() == Street::Preflop,
          "Initial street must be Preflop with button 1");

    check(hand1.pot() == 0,
          "Initial pot must be zero with button 1");

    // Test 3: access player 0
    check(hand0.player(0).stack() == 1000,
          "Player 0 must have the correct stack");

    check(hand0.player(0).holeCards() == holeCards0,
          "Player 0 must have the correct hole cards");

    check(hand0.player(0).committedStreet() == 0,
          "Player 0 must have zero street commitment");

    check(hand0.player(0).committedTotal() == 0,
          "Player 0 must have zero total commitment");

    check(!hand0.player(0).hasFolded(),
          "Player 0 must not be folded");

    // Test 4: access player 1
    check(hand0.player(1).stack() == 1000,
          "Player 1 must have the correct stack");

    check(hand0.player(1).holeCards() == holeCards1,
          "Player 1 must have the correct hole cards");

    check(hand0.player(1).committedStreet() == 0,
          "Player 1 must have zero street commitment");

    check(hand0.player(1).committedTotal() == 0,
          "Player 1 must have zero total commitment");

    check(!hand0.player(1).hasFolded(),
          "Player 1 must not be folded");

    // Test 5: invalid player indices
    bool exceptionThrown = false;

    try {
        (void)hand0.player(2);
    }
    catch (const std::out_of_range&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Player index 2 must throw out_of_range");

    exceptionThrown = false;

    try {
        (void)hand0.player(255);
    }
    catch (const std::out_of_range&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Player index 255 must throw out_of_range");

    // Test 6: invalid button
    exceptionThrown = false;

    try {
        HandState invalid{makePlayers(), 2};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Button index 2 must throw");

    exceptionThrown = false;

    try {
        HandState invalid{makePlayers(), 255};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Button index 255 must throw");

    // Test 7: duplicate hole cards across players
    exceptionThrown = false;

    try {
        std::array<PlayerState, 2> players = {
            PlayerState{1000, holeCards0},
            PlayerState{
                1000,
                {Card{Rank::Ace, Suit::Spades},
                 Card{Rank::Ten, Suit::Clubs}}
            }
        };

        HandState invalid{std::move(players), 0};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Duplicate card across players must throw");

    // Test 8: valid same ranks with different suits
    std::array<PlayerState, 2> sameRankPlayers = {
        PlayerState{
            1000,
            {Card{Rank::Ace, Suit::Spades},
             Card{Rank::Ace, Suit::Hearts}}
        },
        PlayerState{
            1000,
            {Card{Rank::Ace, Suit::Diamonds},
             Card{Rank::Ace, Suit::Clubs}}
        }
    };

    HandState fourAces{std::move(sameRankPlayers), 0};

    check(fourAces.player(0).holeCards()[0] !=
          fourAces.player(1).holeCards()[0],
          "Cards of equal rank and different suits must be distinct");

    check(fourAces.player(0).holeCards()[1] !=
          fourAces.player(1).holeCards()[1],
          "Cards of equal rank and different suits must be distinct");

    // Test 9: reject pre-committed chips
    exceptionThrown = false;

    try {
        auto players = makePlayers();
        players[0].commitChips(100);

        HandState invalid{std::move(players), 0};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Player 0 with committed chips must be rejected");

    exceptionThrown = false;

    try {
        auto players = makePlayers();
        players[1].commitChips(100);
        players[1].beginNewStreet();

        HandState invalid{std::move(players), 0};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Historical committed chips must be rejected");

    // Test 10: reject folded players
    exceptionThrown = false;

    try {
        auto players = makePlayers();
        players[0].fold();

        HandState invalid{std::move(players), 0};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Folded player 0 must be rejected");

    exceptionThrown = false;

    try {
        auto players = makePlayers();
        players[1].fold();

        HandState invalid{std::move(players), 1};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Folded player 1 must be rejected");

    // Test 11: reject total stack overflow
    const auto maxChips =
        std::numeric_limits<std::int64_t>::max();

    exceptionThrown = false;

    try {
        HandState invalid{makePlayers(maxChips, 1), 0};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Total stack exceeding INT64_MAX must throw");

    // Test 12: accept exact maximum stack capacity
    HandState maxHand{makePlayers(maxChips - 1, 1), 0};

    check(maxHand.player(0).stack() == maxChips - 1,
          "Player 0 must retain maximum valid stack");

    check(maxHand.player(1).stack() == 1,
          "Player 1 must retain remaining chip");

    check(maxHand.pot() == 0,
          "Maximum-capacity hand must start with zero pot");

    check(maxHand.player(0).stack() +
          maxHand.player(1).stack() == maxChips,
          "Exact maximum chip capacity must be accepted");

    std::cout << "All HandState tests passed!\n";

    return EXIT_SUCCESS;
}
