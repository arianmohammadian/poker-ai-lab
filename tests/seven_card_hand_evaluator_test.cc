/**
 * @file seven_card_hand_evaluator_test.cc
 * @brief Tests for seven-card poker hand evaluation.
 * @author Arian Mohammadian
 */

#include "poker/card.h"
#include "poker/hand_category.h"
#include "poker/hand_evaluator.h"
#include "poker/hand_rank.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

using poker::Card;
using poker::HandCategory;
using poker::HandEvaluator;
using poker::HandRank;
using poker::Rank;
using poker::Suit;

using Cards7 = std::array<Card, 7>;
using Cards5 = std::array<Card, 5>;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

Card card(int rank, int suit) {
    return Card(static_cast<Rank>(rank), static_cast<Suit>(suit));
}

void expectHand(const Cards7& cards, HandCategory category,
                HandRank::Kickers kickers, const char* message) {
    check(HandEvaluator::evaluate(cards) == HandRank(category, kickers), message);
}

void expectInvalid(const Cards7& cards, const char* message) {
    try {
        (void)HandEvaluator::evaluate(cards);
    } catch (const std::invalid_argument&) {
        return;
    }
    check(false, message);
}

void testCategoriesAndKickers() {
    expectHand({card(14, 0), card(13, 1), card(11, 2), card(9, 3),
                card(7, 0), card(5, 1), card(2, 2)},
               HandCategory::HighCard, {14, 13, 11, 9, 7}, "Seven-card high card");

    expectHand({card(8, 0), card(8, 1), card(14, 2), card(13, 3),
                card(12, 0), card(9, 1), card(3, 2)},
               HandCategory::OnePair, {8, 14, 13, 12, 0}, "Seven-card pair kickers");

    expectHand({card(14, 0), card(14, 1), card(13, 0), card(13, 1),
                card(12, 2), card(11, 3), card(2, 0)},
               HandCategory::TwoPair, {14, 13, 12, 0, 0}, "Seven-card two pair");

    expectHand({card(7, 0), card(7, 1), card(7, 2), card(14, 3),
                card(13, 0), card(9, 1), card(2, 2)},
               HandCategory::ThreeOfAKind, {7, 14, 13, 0, 0}, "Seven-card trips");

    expectHand({card(10, 0), card(9, 1), card(8, 2), card(7, 3),
                card(6, 0), card(5, 1), card(4, 2)},
               HandCategory::Straight, {10, 0, 0, 0, 0}, "Highest overlapping straight");

    expectHand({card(14, 0), card(13, 0), card(11, 0), card(9, 0),
                card(6, 0), card(3, 0), card(2, 1)},
               HandCategory::Flush, {14, 13, 11, 9, 6}, "Best five of six suited cards");

    expectHand({card(13, 0), card(13, 1), card(13, 2), card(8, 0),
                card(8, 1), card(8, 2), card(14, 3)},
               HandCategory::FullHouse, {13, 8, 0, 0, 0}, "Two trips make best full house");

    expectHand({card(9, 0), card(9, 1), card(9, 2), card(9, 3),
                card(14, 0), card(13, 1), card(2, 2)},
               HandCategory::FourOfAKind, {9, 14, 0, 0, 0}, "Quads with highest kicker");

    expectHand({card(14, 0), card(13, 0), card(12, 0), card(11, 0),
                card(10, 0), card(2, 1), card(3, 2)},
               HandCategory::StraightFlush, {14, 0, 0, 0, 0}, "Royal flush among seven");
}

void testTrickySelections() {
    expectHand({card(14, 0), card(14, 1), card(13, 0), card(13, 1),
                card(12, 0), card(12, 1), card(2, 2)},
               HandCategory::TwoPair, {14, 13, 12, 0, 0}, "Three pairs: choose top two");

    expectHand({card(14, 0), card(14, 1), card(14, 2), card(13, 0),
                card(13, 1), card(12, 0), card(12, 1)},
               HandCategory::FullHouse, {14, 13, 0, 0, 0}, "Three plus two pairs");

    expectHand({card(14, 0), card(5, 1), card(4, 2), card(3, 3),
                card(2, 0), card(13, 1), card(12, 2)},
               HandCategory::Straight, {5, 0, 0, 0, 0}, "Wheel with extra high cards");

    expectHand({card(14, 2), card(5, 2), card(4, 2), card(3, 2),
                card(2, 2), card(13, 0), card(12, 1)},
               HandCategory::StraightFlush, {5, 0, 0, 0, 0}, "Wheel straight flush");

    expectHand({card(14, 0), card(13, 0), card(11, 0), card(9, 0),
                card(6, 0), card(3, 0), card(2, 0)},
               HandCategory::Flush, {14, 13, 11, 9, 6}, "Seven cards same suit");

    expectHand({card(7, 0), card(7, 1), card(7, 2), card(8, 3),
                card(9, 0), card(10, 1), card(11, 2)},
               HandCategory::Straight, {11, 0, 0, 0, 0}, "Straight beats trips");

    expectHand({card(9, 0), card(8, 0), card(7, 0), card(6, 0),
                card(5, 0), card(14, 1), card(13, 2)},
               HandCategory::StraightFlush, {9, 0, 0, 0, 0}, "Straight flush beats high cards");

    expectHand({card(14, 0), card(13, 0), card(11, 0), card(8, 0),
                card(3, 0), card(12, 1), card(10, 2)},
               HandCategory::Flush, {14, 13, 11, 8, 3}, "Flush beats non-flush straight prospects");
}

void testDuplicates() {
    expectInvalid({card(14, 0), card(13, 1), card(12, 2), card(11, 3),
                   card(10, 0), card(9, 1), card(14, 0)},
                  "Duplicate first and last cards must throw");
    expectInvalid({card(8, 0), card(8, 1), card(8, 2), card(8, 3),
                   card(8, 0), card(14, 1), card(13, 2)},
                  "Duplicate amid four of a kind must throw");
}

void testPermutationIndependence() {
    const std::array<Cards7, 3> hands{{
        {card(14, 0), card(14, 1), card(13, 0), card(13, 1),
         card(12, 0), card(12, 1), card(2, 2)},
        {card(13, 0), card(13, 1), card(13, 2), card(8, 0),
         card(8, 1), card(8, 2), card(14, 3)},
        {card(14, 2), card(5, 2), card(4, 2), card(3, 2),
         card(2, 2), card(13, 0), card(12, 1)}
    }};

    for (const Cards7& hand : hands) {
        const HandRank expected = HandEvaluator::evaluate(hand);
        std::array<std::size_t, 7> order{0, 1, 2, 3, 4, 5, 6};
        std::size_t permutations = 0;
        do {
            const Cards7 reordered{
                hand[order[0]], hand[order[1]], hand[order[2]],
                hand[order[3]], hand[order[4]], hand[order[5]], hand[order[6]]
            };
            check(HandEvaluator::evaluate(reordered) == expected,
                  "Seven-card evaluation must be order-independent");
            ++permutations;
        } while (std::next_permutation(order.begin(), order.end()));
        check(permutations == 5040, "All seven-card permutations tested");
    }
}

// Independent reference enumeration: choose five indices rather than exclude two.
// This checks the seven-card combination generation against the trusted five-card evaluator.
HandRank referenceBestFive(const Cards7& cards) {
    HandRank best = HandEvaluator::evaluate(Cards5{
        cards[0], cards[1], cards[2], cards[3], cards[4]
    });

    for (std::size_t a = 0; a < 3; ++a)
    for (std::size_t b = a + 1; b < 4; ++b)
    for (std::size_t c = b + 1; c < 5; ++c)
    for (std::size_t d = c + 1; d < 6; ++d)
    for (std::size_t e = d + 1; e < 7; ++e) {
        const HandRank candidate = HandEvaluator::evaluate(Cards5{
            cards[a], cards[b], cards[c], cards[d], cards[e]
        });
        if (candidate > best) best = candidate;
    }
    return best;
}

void testDeterministicRandomHands() {
    std::vector<Card> deck;
    deck.reserve(52);
    for (int rank = 2; rank <= 14; ++rank)
        for (int suit = 0; suit < 4; ++suit)
            deck.push_back(card(rank, suit));

    std::mt19937 rng(20261104);
    for (int trial = 0; trial < 1000; ++trial) {
        std::shuffle(deck.begin(), deck.end(), rng);
        const Cards7 cards{
            deck[0], deck[1], deck[2], deck[3],
            deck[4], deck[5], deck[6]
        };
        check(HandEvaluator::evaluate(cards) == referenceBestFive(cards),
              "Seven-card evaluation must match five-card reference enumeration");
    }
}

} // namespace

int main() {
    testCategoriesAndKickers();
    testTrickySelections();
    testDuplicates();
    testPermutationIndependence();
    testDeterministicRandomHands();

    std::cout << "All seven-card HandEvaluator tests passed.\n";
    return EXIT_SUCCESS;
}
