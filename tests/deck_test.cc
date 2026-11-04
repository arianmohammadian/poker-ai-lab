
#include "poker/deck.h"

#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <set>

using poker::Card;
using poker::Deck;
using poker::Rank;
using poker::Suit;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    // Test 1: initial state
    {
        Deck deck;

        check(deck.remaining() == Deck::kDeckSize, "Initial Deck must have 52 cards");
    }

    // Test 2: drawing reduces remaining cards
    {
        Deck deck;

        Card c1 = deck.draw();
        check(deck.remaining() == 51, "Deck must have 51 remaining cards after 1 draw");

        Card c2 = deck.draw();
        check(deck.remaining() == 50, "Deck must have 50 remaining cards after 2 draws");

        check(c1 != c2, "Consecutive draws must return different cards");
    }

    // Test 3: all 52 cards are unique
    {
        Deck deck;

        std::set<std::pair<int, int>> drawn_cards;
        for(std::size_t i = 0; i < Deck::kDeckSize; i++) {
            Card card = deck.draw();

            int rank = static_cast<int>(card.rank());
            int suit = static_cast<int>(card.suit());

            drawn_cards.insert({rank, suit});
        }

        check(drawn_cards.size() == 52, "Deck must not have duplicated cards");
        check(deck.remaining() == 0, "Deck must have 0 remaining cards after drawing all of them");
    }

    // Test 4: drawing from an empty deck throws
    {
        Deck deck;

        for(std::size_t i = 0; i < Deck::kDeckSize; i++) {
            (void)deck.draw();
        }

        bool exceptionThrownDraw = false;

        try {
            (void)deck.draw();
        }
        catch (const std::out_of_range& e) {
            exceptionThrownDraw = true;
        }

        check(exceptionThrownDraw, "Drawing from an empty deck must throw");
        check(deck.remaining() == 0, "Failed draw must not change remaining cards");
    }

    // Test 5: reset restores the original sequence
    {
        Deck deck;

        std::mt19937 rng{52};

        deck.shuffle(rng);
        
        std::vector<Card> drawn1, drawn2;

        for(int i = 0; i < 4; i++) {
            drawn1.push_back(deck.draw());
        }

        deck.reset();

        check(deck.remaining() == Deck::kDeckSize, "Reseted deck must have 52 remaining cards");

        for(int i = 0; i < 4; i++) {
            drawn2.push_back(deck.draw());
        }
        

        check(drawn1 == drawn2, "A sequence of drawn cards must be the same before and after resetting the deck");
    }

    // Test 6: shuffle preserves all 52 unique cards
    {
        Deck shuffled, original;
        std::mt19937 rng{42};

        std::set<std::pair<int, int>> shuffled_cards, original_cards;

        shuffled.shuffle(rng);

        check(shuffled.remaining() == Deck::kDeckSize, "Shuffle must leave all 52 cards available");

        for(std::size_t i = 0; i < Deck::kDeckSize; i++) {
            Card card1 = shuffled.draw(), card2 = original.draw();

            int rank1 = static_cast<int>(card1.rank());
            int suit1 = static_cast<int>(card1.suit());

            int rank2 = static_cast<int>(card2.rank());
            int suit2 = static_cast<int>(card2.suit());

            shuffled_cards.insert({rank1, suit1});
            original_cards.insert({rank2, suit2});
        }

        check(shuffled.remaining() == 0, "Shuffled deck must be empty after 52 draws");
        check(shuffled_cards == original_cards, "A shuffled deck must have the same cards as the original one but in an arbitrary (and probably different) order");
    
        check(shuffled_cards.size() == Deck::kDeckSize, "Shuffled deck must contain 52 unique cards");
        check(original_cards.size() == Deck::kDeckSize, "Original deck must contain 52 unique cards");
    }

    // Test 7: shuffle is reproducible
    {
        Deck deck1;
        Deck deck2;

        std::mt19937 rng1{42};
        std::mt19937 rng2{42};

        deck1.shuffle(rng1);
        deck2.shuffle(rng2);

        for(int i = 0; i < Deck::kDeckSize; i++) {
            Card card1 = deck1.draw();
            Card card2 = deck2.draw();

            check(card1 == card2, "Sequence of drawn cards must be the same for decks that have been shuffled with the same generator");
        }
    }

    // Test 8: shuffle restores a partially dealt deck
    {
        Deck deck;
        std::mt19937 rng{42};

        for(int i = 0; i < 10; i++) {
            (void)deck.draw();
        }

        check(deck.remaining() == 42, "Deck must have 42 remaining cards after 10 draws");

        deck.shuffle(rng);

        check(deck.remaining() == Deck::kDeckSize, "Deck must have 52 remaining cards after being shuffled");
    }

    // Test 9: deck copy preserves independent state
    {
        Deck original;

        (void)original.draw();
        (void)original.draw();

        Deck copy = original;

        check(original.remaining() == copy.remaining() && original.remaining() == 50, "Copied deck must have the same state as the original one");

        (void)copy.draw();

        check(original.remaining() == 50, "Original deck must not change after changing a copied deck");
        check(copy.remaining() == 49, "Copied deck must behave correctly after any operation applied to it");
    }

    std::cout << "All Deck tests passed!\n";

    return EXIT_SUCCESS;
}
