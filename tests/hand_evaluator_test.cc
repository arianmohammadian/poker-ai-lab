/**
 * @file hand_evaluator_test.cc
 * @brief Unit and exhaustive tests for five-card hand evaluation.
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
#include <stdexcept>
#include <vector>

namespace {

using poker::Card;
using poker::HandCategory;
using poker::HandEvaluator;
using poker::HandRank;
using poker::Rank;
using poker::Suit;

using Cards = std::array<Card, 5>;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

Card card(int rank, int suit) {
    return Card(static_cast<Rank>(rank), static_cast<Suit>(suit));
}

void expectHand(const Cards& cards,
                HandCategory category,
                HandRank::Kickers kickers,
                const char* message) {
    const HandRank actual = HandEvaluator::evaluate(cards);
    const HandRank expected(category, kickers);
    check(actual == expected, message);
}

void expectInvalid(const Cards& cards, const char* message) {
    try {
        (void)HandEvaluator::evaluate(cards);
    } catch (const std::invalid_argument&) {
        return;
    }
    check(false, message);
}

void testCategoriesAndKickers() {
    // Suit encoding is irrelevant to relative strength: 0..3 are four distinct suits.
    expectHand({card(14, 0), card(13, 1), card(11, 2), card(8, 3), card(3, 0)},
               HandCategory::HighCard, {14, 13, 11, 8, 3}, "High card");

    expectHand({card(8, 0), card(8, 1), card(14, 2), card(12, 3), card(10, 0)},
               HandCategory::OnePair, {8, 14, 12, 10, 0}, "One pair and kickers");

    // An ace can be the kicker even when the pair ranks are lower.
    expectHand({card(10, 0), card(10, 1), card(8, 2), card(8, 3), card(14, 0)},
               HandCategory::TwoPair, {10, 8, 14, 0, 0}, "Two pairs, ace kicker");

    expectHand({card(7, 0), card(7, 1), card(7, 2), card(14, 3), card(12, 0)},
               HandCategory::ThreeOfAKind, {7, 14, 12, 0, 0}, "Three of a kind");

    expectHand({card(14, 0), card(13, 1), card(12, 2), card(11, 3), card(10, 0)},
               HandCategory::Straight, {14, 0, 0, 0, 0}, "Broadway straight");

    expectHand({card(14, 0), card(5, 1), card(4, 2), card(3, 3), card(2, 0)},
               HandCategory::Straight, {5, 0, 0, 0, 0}, "Wheel straight");

    expectHand({card(14, 0), card(12, 0), card(9, 0), card(6, 0), card(3, 0)},
               HandCategory::Flush, {14, 12, 9, 6, 3}, "Flush with five kickers");

    expectHand({card(13, 0), card(13, 1), card(13, 2), card(8, 3), card(8, 0)},
               HandCategory::FullHouse, {13, 8, 0, 0, 0}, "Full house");

    expectHand({card(10, 0), card(10, 1), card(10, 2), card(10, 3), card(14, 0)},
               HandCategory::FourOfAKind, {10, 14, 0, 0, 0}, "Four of a kind");

    expectHand({card(14, 1), card(13, 1), card(12, 1), card(11, 1), card(10, 1)},
               HandCategory::StraightFlush, {14, 0, 0, 0, 0}, "Royal flush");

    expectHand({card(14, 2), card(5, 2), card(4, 2), card(3, 2), card(2, 2)},
               HandCategory::StraightFlush, {5, 0, 0, 0, 0}, "Wheel straight flush");

    // Straights must contain five distinct ranks; pairs don't qualify.
    expectHand({card(14, 0), card(14, 1), card(13, 2), card(12, 3), card(11, 0)},
               HandCategory::OnePair, {14, 13, 12, 11, 0}, "Pair is not a straight");
}

void testDuplicateCards() {
    expectInvalid({card(14, 0), card(14, 0), card(13, 1), card(12, 2), card(11, 3)},
                  "Duplicate ace of the same suit must be rejected");
    expectInvalid({card(2, 0), card(3, 1), card(4, 2), card(5, 3), card(2, 0)},
                  "Duplicate in first and last positions must be rejected");

    // Same rank with different suits is legal.
    expectHand({card(14, 0), card(14, 1), card(13, 2), card(12, 3), card(11, 0)},
               HandCategory::OnePair, {14, 13, 12, 11, 0},
               "Same rank, different suit is allowed");
}

void testOrderIndependence() {
    // Every permutation of each representative hand must produce the same rank.
    const std::array<Cards, 9> representatives{{
        {card(14, 0), card(13, 1), card(11, 2), card(8, 3), card(3, 0)},
        {card(8, 0), card(8, 1), card(14, 2), card(12, 3), card(10, 0)},
        {card(10, 0), card(10, 1), card(8, 2), card(8, 3), card(14, 0)},
        {card(7, 0), card(7, 1), card(7, 2), card(14, 3), card(12, 0)},
        {card(14, 0), card(5, 1), card(4, 2), card(3, 3), card(2, 0)},
        {card(14, 0), card(12, 0), card(9, 0), card(6, 0), card(3, 0)},
        {card(13, 0), card(13, 1), card(13, 2), card(8, 3), card(8, 0)},
        {card(10, 0), card(10, 1), card(10, 2), card(10, 3), card(14, 0)},
        {card(14, 1), card(13, 1), card(12, 1), card(11, 1), card(10, 1)}
    }};

    for (const Cards& original : representatives) {
        const HandRank expected = HandEvaluator::evaluate(original);
        std::array<std::size_t, 5> permutation{0, 1, 2, 3, 4};
        do {
            const Cards shuffled{
                original[permutation[0]], original[permutation[1]],
                original[permutation[2]], original[permutation[3]],
                original[permutation[4]]
            };
            check(HandEvaluator::evaluate(shuffled) == expected,
                  "Evaluation must not depend on card order");
        } while (std::next_permutation(permutation.begin(), permutation.end()));
    }
}

void testComparisons() {
    const HandRank onePairA = HandEvaluator::evaluate(
        {card(14, 0), card(14, 1), card(13, 2), card(12, 3), card(9, 0)});
    const HandRank onePairB = HandEvaluator::evaluate(
        {card(14, 2), card(14, 3), card(13, 0), card(11, 1), card(10, 2)});
    check(onePairA > onePairB, "Second kicker decides equal pairs");

    const HandRank flushA = HandEvaluator::evaluate(
        {card(14, 0), card(13, 0), card(11, 0), card(8, 0), card(3, 0)});
    const HandRank flushB = HandEvaluator::evaluate(
        {card(14, 1), card(13, 1), card(10, 1), card(9, 1), card(5, 1)});
    check(flushA > flushB, "Flush comparison uses all five ranks");

    const HandRank sameFlush = HandEvaluator::evaluate(
        {card(14, 2), card(13, 2), card(11, 2), card(8, 2), card(3, 2)});
    check(flushA == sameFlush, "Suit does not break ties");

    const HandRank wheel = HandEvaluator::evaluate(
        {card(14, 0), card(5, 1), card(4, 2), card(3, 3), card(2, 0)});
    const HandRank sixHigh = HandEvaluator::evaluate(
        {card(6, 0), card(5, 1), card(4, 2), card(3, 3), card(2, 0)});
    check(sixHigh > wheel, "Six-high straight beats wheel");
}

void testExhaustiveCounts() {
    std::vector<Card> deck;
    deck.reserve(52);
    for (int rank = 2; rank <= 14; ++rank) {
        for (int suit = 0; suit < 4; ++suit) {
            deck.push_back(card(rank, suit));
        }
    }
    check(deck.size() == 52, "Reference deck contains 52 cards");

    std::array<std::uint64_t, 9> counts{};
    std::array<std::uint64_t, 15> straightHighCounts{};
    std::array<std::uint64_t, 15> straightFlushHighCounts{};
    std::uint64_t total = 0;

    // Strictly increasing indices enumerate each unordered 5-card hand once.
    for (std::size_t a = 0; a < 48; ++a)
    for (std::size_t b = a + 1; b < 49; ++b)
    for (std::size_t c = b + 1; c < 50; ++c)
    for (std::size_t d = c + 1; d < 51; ++d)
    for (std::size_t e = d + 1; e < 52; ++e) {
        const Cards cards{deck[a], deck[b], deck[c], deck[d], deck[e]};
        const HandRank result = HandEvaluator::evaluate(cards);
        const std::size_t category = static_cast<std::size_t>(result.category());
        check(category < counts.size(), "Evaluator produced an invalid category");
        ++counts[category];
        ++total;

        if (result.category() == HandCategory::Straight)
            ++straightHighCounts[result.kickers()[0]];
        if (result.category() == HandCategory::StraightFlush)
            ++straightFlushHighCounts[result.kickers()[0]];
    }

    const std::array<std::uint64_t, 9> expected{
        1302540, 1098240, 123552, 54912, 10200,
        5108, 3744, 624, 40
    };
    const std::array<const char*, 9> names{
        "HighCard", "OnePair", "TwoPair", "ThreeOfAKind", "Straight",
        "Flush", "FullHouse", "FourOfAKind", "StraightFlush"
    };

    check(total == 2598960, "Exactly C(52,5) hands must be evaluated");
    for (std::size_t i = 0; i < counts.size(); ++i) {
        if (counts[i] != expected[i]) {
            std::cerr << "FAILED: Exhaustive count for " << names[i]
                      << ": expected " << expected[i]
                      << ", got " << counts[i] << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    // Each of the ten straight heights occurs in 1,020 non-flush hands
    // and four straight flush hands (one per suit).
    for (std::size_t high = 5; high <= 14; ++high) {
        check(straightHighCounts[high] == 1020,
              "Each straight height occurs exactly 1,020 times");
        check(straightFlushHighCounts[high] == 4,
              "Each straight-flush height occurs exactly four times");
    }

    std::cout << "Exhaustive five-card validation passed: "
              << total << " combinations.\n";
}

} // namespace

int main() {
    testCategoriesAndKickers();
    testDuplicateCards();
    testOrderIndependence();
    testComparisons();
    testExhaustiveCounts();

    std::cout << "All HandEvaluator tests passed.\n";
    return EXIT_SUCCESS;
}
