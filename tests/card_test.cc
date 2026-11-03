
#include "poker/card.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <type_traits>

using poker::Card;
using poker::Rank;
using poker::Suit;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

// Compile-time test
static_assert(!std::is_default_constructible_v<Card>);

int main() {
    // Test 1: valid construction
    Card ace{Rank::Ace, Suit::Spades};

    check(ace.rank() == Rank::Ace, "Ace must have rank Ace");
    check(ace.suit() == Suit::Spades, "Ace must have suit Spades");

    // Test 2: equality
    Card ace1{Rank::Ace, Suit::Spades};
    Card ace2{Rank::Ace, Suit::Spades};

    check(ace1 == ace2, "Identical cards must be equal");

    // Test 3: inequality
    Card aceSpades{Rank::Ace, Suit::Spades};
    Card aceClubs{Rank::Ace, Suit::Clubs};
    Card kingSpades{Rank::King, Suit::Spades};

    check(aceSpades != aceClubs,
          "Cards with different suits must not be equal");

    check(aceSpades != kingSpades,
          "Cards with different ranks must not be equal");

    // Test 4: invalid rank
    bool exceptionThrownRank = false;

    try {
        Card invalid{static_cast<Rank>(1), Suit::Hearts};
    }
    catch (const std::invalid_argument& e) {
        exceptionThrownRank = true;
    }

    check(exceptionThrownRank, "Rank below Two must throw");

    exceptionThrownRank = false;

    try {
        Card invalid{static_cast<Rank>(15), Suit::Hearts};
    }
    catch (const std::invalid_argument& e) {
        exceptionThrownRank = true;
    }

    check(exceptionThrownRank, "Rank above Ace must throw");

    // Test 5: invalid suit
    bool exceptionThrownSuit = false;

    try {
        Card invalid{Rank::Ace, static_cast<Suit>(4)};
    }
    catch (const std::invalid_argument& e) {
        exceptionThrownSuit = true;
    }

    check(exceptionThrownSuit, "Suit above Spades must throw");

    // Test 6: inclusive limits
    Card lowest{Rank::Two, Suit::Clubs};
    Card highest{Rank::Ace, Suit::Spades};

    check(lowest.rank() == Rank::Two,
          "Lowest rank must be Two");

    check(lowest.suit() == Suit::Clubs,
          "Lowest suit must be Clubs");

    check(highest.rank() == Rank::Ace,
          "Highest rank must be Ace");

    check(highest.suit() == Suit::Spades,
          "Highest suit must be Spades");

    std::cout << "All Card tests passed!\n";

    return EXIT_SUCCESS;
}
