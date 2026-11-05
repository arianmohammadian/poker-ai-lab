
#include "poker/player_state.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

using poker::Card;
using poker::PlayerState;
using poker::Rank;
using poker::Suit;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

// Compile-time tests
static_assert(!std::is_default_constructible_v<PlayerState>);

static_assert(std::is_same_v<
    decltype(std::declval<const PlayerState&>().holeCards()),
    const std::array<Card, 2>&
>);

int main() {
    const std::array<Card, 2> holeCards = {
        Card{Rank::Ace, Suit::Spades},
        Card{Rank::King, Suit::Hearts}
    };

    // Test 1: valid construction
    PlayerState player{1000, holeCards};

    check(player.stack() == 1000,
          "Initial stack must be 1000");

    check(player.committedStreet() == 0,
          "Initial street commitment must be zero");

    check(player.committedTotal() == 0,
          "Initial total commitment must be zero");

    check(!player.hasFolded(),
          "Player must not be folded initially");

    check(!player.isAllIn(),
          "Player must not be all-in initially");

    check(player.holeCards() == holeCards,
          "Hole cards must match the provided cards");

    // Test 2: negative initial stack
    bool exceptionThrown = false;

    try {
        PlayerState invalid{-100, holeCards};
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Negative initial stack must throw");

    // Test 3: duplicated hole cards
    exceptionThrown = false;

    try {
        PlayerState invalid{
            1000,
            {Card{Rank::Ace, Suit::Spades},
             Card{Rank::Ace, Suit::Spades}}
        };
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Duplicated hole cards must throw");

    // Test 4: zero initial stack
    PlayerState emptyPlayer{0, holeCards};

    check(emptyPlayer.stack() == 0,
          "Zero initial stack must be allowed");

    check(emptyPlayer.isAllIn(),
          "Active player with zero stack must be all-in");

    check(!emptyPlayer.hasFolded(),
          "Zero-stack player must not be folded");

    // Test 5: valid chip commitments
    PlayerState bettor{1000, holeCards};

    bettor.commitChips(100);

    check(bettor.stack() == 900,
          "Stack must decrease after committing chips");

    check(bettor.committedStreet() == 100,
          "Street commitment must increase");

    check(bettor.committedTotal() == 100,
          "Total commitment must increase");

    bettor.commitChips(250);

    check(bettor.stack() == 650,
          "Stack must reflect multiple commitments");

    check(bettor.committedStreet() == 350,
          "Street commitment must accumulate");

    check(bettor.committedTotal() == 350,
          "Total commitment must accumulate");

    check(bettor.stack() + bettor.committedTotal() == 1000,
          "Chip conservation must hold after commitments");

    // Test 6: invalid chip amounts
    PlayerState invalidBettor{1000, holeCards};

    exceptionThrown = false;

    try {
        invalidBettor.commitChips(-100);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Negative commitment must throw");

    exceptionThrown = false;

    try {
        invalidBettor.commitChips(0);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Zero commitment must throw");

    exceptionThrown = false;

    try {
        invalidBettor.commitChips(1001);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Commitment exceeding stack must throw");

    // Test 7: state unchanged after invalid commitments
    check(invalidBettor.stack() == 1000,
          "Invalid commitments must not modify stack");

    check(invalidBettor.committedStreet() == 0,
          "Invalid commitments must not modify street commitment");

    check(invalidBettor.committedTotal() == 0,
          "Invalid commitments must not modify total commitment");

    // Test 8: exact all-in
    PlayerState allInPlayer{500, holeCards};

    allInPlayer.commitChips(500);

    check(allInPlayer.stack() == 0,
          "All-in must leave zero chips");

    check(allInPlayer.committedStreet() == 500,
          "All-in must commit the entire stack");

    check(allInPlayer.committedTotal() == 500,
          "All-in must update total commitment");

    check(allInPlayer.isAllIn(),
          "Player with zero stack must be all-in");

    check(!allInPlayer.hasFolded(),
          "All-in player must not be folded");

    // Test 9: cannot commit more chips after all-in
    exceptionThrown = false;

    try {
        allInPlayer.commitChips(1);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "All-in player cannot commit additional chips");

    check(allInPlayer.stack() == 0 &&
          allInPlayer.committedTotal() == 500,
          "Failed all-in commitment must preserve state");

    // Test 10: fold and repeated fold
    PlayerState foldedPlayer{1000, holeCards};

    foldedPlayer.commitChips(100);
    foldedPlayer.fold();

    check(foldedPlayer.hasFolded(),
          "Player must be folded after fold()");

    check(!foldedPlayer.isAllIn(),
          "Folded player must not be all-in");

    check(foldedPlayer.stack() == 900,
          "Folding must not modify stack");

    check(foldedPlayer.committedTotal() == 100,
          "Folding must preserve committed chips");

    foldedPlayer.fold();

    check(foldedPlayer.hasFolded(),
          "Repeated fold must be idempotent");

    // Test 11: cannot commit chips after folding
    exceptionThrown = false;

    try {
        foldedPlayer.commitChips(200);
    }
    catch (const std::logic_error&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Folded player cannot commit chips");

    check(foldedPlayer.stack() == 900,
          "Failed commitment after fold must preserve stack");

    check(foldedPlayer.committedStreet() == 100,
          "Failed commitment after fold must preserve street commitment");

    check(foldedPlayer.committedTotal() == 100,
          "Failed commitment after fold must preserve total commitment");

    // Test 12: transition between streets
    PlayerState streetPlayer{1000, holeCards};

    streetPlayer.commitChips(100);
    streetPlayer.beginNewStreet();

    check(streetPlayer.stack() == 900,
          "New street must not modify stack");

    check(streetPlayer.committedStreet() == 0,
          "New street must reset street commitment");

    check(streetPlayer.committedTotal() == 100,
          "New street must preserve total commitment");

    streetPlayer.commitChips(200);

    check(streetPlayer.stack() == 700,
          "Stack must reflect commitments across streets");

    check(streetPlayer.committedStreet() == 200,
          "Street commitment must count only current street");

    check(streetPlayer.committedTotal() == 300,
          "Total commitment must accumulate across streets");

    check(streetPlayer.stack() + streetPlayer.committedTotal() == 1000,
          "Chip conservation must hold across streets");

    streetPlayer.beginNewStreet();
    streetPlayer.beginNewStreet();

    check(streetPlayer.committedStreet() == 0,
          "Repeated street reset must preserve zero commitment");

    check(streetPlayer.committedTotal() == 300,
          "Repeated street reset must preserve total commitment");

    std::cout << "All PlayerState tests passed!\n";

    return EXIT_SUCCESS;
}
