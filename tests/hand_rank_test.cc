
#include "poker/hand_rank.h"

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>

using poker::HandCategory;
using poker::HandRank;

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void expectInvalid(HandCategory category,
                   HandRank::Kickers kickers,
                   const char* message) {
    try {
        HandRank rank(category, kickers);
        (void)rank;
    } catch (const std::invalid_argument&) {
        return;
    }

    check(false, message);
}

} // namespace

static_assert(!std::is_default_constructible_v<HandRank>);

int main() {

    // TEST 1: All nine categories accept valid representations.
    {
        HandRank high(HandCategory::HighCard, {14, 13, 11, 8, 3});
        HandRank pair(HandCategory::OnePair, {8, 14, 12, 10, 0});
        HandRank twoPair(HandCategory::TwoPair, {10, 8, 14, 0, 0});
        HandRank trips(HandCategory::ThreeOfAKind, {7, 14, 12, 0, 0});
        HandRank straight(HandCategory::Straight, {10, 0, 0, 0, 0});
        HandRank flush(HandCategory::Flush, {14, 12, 9, 6, 3});
        HandRank full(HandCategory::FullHouse, {13, 8, 0, 0, 0});
        HandRank quads(HandCategory::FourOfAKind, {10, 14, 0, 0, 0});
        HandRank straightFlush(HandCategory::StraightFlush,
                               {14, 0, 0, 0, 0});

        check(high.category() == HandCategory::HighCard,
              "HighCard category");
        check(pair.category() == HandCategory::OnePair,
              "OnePair category");
        check(twoPair.category() == HandCategory::TwoPair,
              "TwoPair category");
        check(trips.category() == HandCategory::ThreeOfAKind,
              "ThreeOfAKind category");
        check(straight.category() == HandCategory::Straight,
              "Straight category");
        check(flush.category() == HandCategory::Flush,
              "Flush category");
        check(full.category() == HandCategory::FullHouse,
              "FullHouse category");
        check(quads.category() == HandCategory::FourOfAKind,
              "FourOfAKind category");
        check(straightFlush.category() == HandCategory::StraightFlush,
              "StraightFlush category");
    }

    // TEST 2: Getters expose the correct values.
    {
        HandRank rank(HandCategory::OnePair, {8, 14, 12, 10, 0});

        check(rank.category() == HandCategory::OnePair,
              "Category getter");
        check(rank.kickers() == HandRank::Kickers{8, 14, 12, 10, 0},
              "Kickers getter");
    }

    // TEST 3: Invalid category and out-of-range values.
    {
        expectInvalid(static_cast<HandCategory>(9),
                      {14, 13, 11, 8, 3},
                      "Category above maximum must be rejected");

        expectInvalid(static_cast<HandCategory>(255),
                      {14, 13, 11, 8, 3},
                      "Invalid enum value must be rejected");

        expectInvalid(HandCategory::OnePair,
                      {15, 14, 12, 10, 0},
                      "Rank above Ace must be rejected");

        expectInvalid(HandCategory::Straight,
                      {4, 0, 0, 0, 0},
                      "Straight below five must be rejected");
    }

    // TEST 4: Invalid pair representations.
    {
        expectInvalid(HandCategory::OnePair,
                      {14, 14, 12, 10, 0},
                      "Pair rank cannot equal a kicker");

        expectInvalid(HandCategory::OnePair,
                      {8, 10, 12, 9, 0},
                      "Pair kickers must be descending");

        expectInvalid(HandCategory::OnePair,
                      {8, 14, 12, 10, 2},
                      "Pair unused position must be zero");

        expectInvalid(HandCategory::TwoPair,
                      {8, 10, 14, 0, 0},
                      "TwoPair must store higher pair first");

        expectInvalid(HandCategory::TwoPair,
                      {10, 8, 10, 0, 0},
                      "TwoPair kicker cannot equal a pair");

        expectInvalid(HandCategory::TwoPair,
                      {10, 8, 8, 0, 0},
                      "TwoPair kicker cannot equal lower pair");
    }

    // TEST 5: Invalid trips, full house and four of a kind.
    {
        expectInvalid(HandCategory::ThreeOfAKind,
                      {7, 7, 12, 0, 0},
                      "Trips kicker cannot equal trips rank");

        expectInvalid(HandCategory::ThreeOfAKind,
                      {7, 10, 14, 0, 0},
                      "Trips kickers must be descending");

        expectInvalid(HandCategory::FullHouse,
                      {14, 14, 0, 0, 0},
                      "FullHouse ranks must be different");

        expectInvalid(HandCategory::FourOfAKind,
                      {10, 10, 0, 0, 0},
                      "Quads kicker must differ from quads rank");

        expectInvalid(HandCategory::FullHouse,
                      {13, 8, 2, 0, 0},
                      "FullHouse unused positions must be zero");
    }

    // TEST 6: Invalid HighCard and Flush representations.
    {
        expectInvalid(HandCategory::HighCard,
                      {14, 14, 11, 8, 3},
                      "HighCard cannot contain repeated ranks");

        expectInvalid(HandCategory::HighCard,
                      {14, 13, 12, 11, 10},
                      "Broadway straight is not HighCard");

        expectInvalid(HandCategory::HighCard,
                      {14, 5, 4, 3, 2},
                      "Wheel straight is not HighCard");

        expectInvalid(HandCategory::Flush,
                      {14, 13, 12, 11, 10},
                      "Straight flush cannot be classified as Flush");

        expectInvalid(HandCategory::Flush,
                      {14, 5, 4, 3, 2},
                      "Wheel straight flush cannot be Flush");

        expectInvalid(HandCategory::Flush,
                      {14, 12, 9, 9, 3},
                      "Flush cannot contain repeated ranks");

        expectInvalid(HandCategory::HighCard,
                      {14, 13, 11, 8, 0},
                      "HighCard must contain five valid ranks");
    }

    // TEST 7: Straight and StraightFlush boundaries.
    {
        HandRank wheel(HandCategory::Straight, {5, 0, 0, 0, 0});
        HandRank broadway(HandCategory::Straight, {14, 0, 0, 0, 0});
        HandRank royal(HandCategory::StraightFlush, {14, 0, 0, 0, 0});

        check(wheel < broadway,
              "Five-high straight loses to Ace-high straight");

        check(broadway < royal,
              "StraightFlush beats Straight");

        expectInvalid(HandCategory::Straight,
                      {15, 0, 0, 0, 0},
                      "Straight above Ace must be rejected");

        expectInvalid(HandCategory::StraightFlush,
                      {10, 9, 0, 0, 0},
                      "StraightFlush unused positions must be zero");
    }

    // TEST 8: All categories have the correct strength order.
    {
        const HandRank ranks[] = {
            {HandCategory::HighCard, {14, 13, 11, 8, 3}},
            {HandCategory::OnePair, {8, 14, 12, 10, 0}},
            {HandCategory::TwoPair, {10, 8, 14, 0, 0}},
            {HandCategory::ThreeOfAKind, {7, 14, 12, 0, 0}},
            {HandCategory::Straight, {10, 0, 0, 0, 0}},
            {HandCategory::Flush, {14, 12, 9, 6, 3}},
            {HandCategory::FullHouse, {13, 8, 0, 0, 0}},
            {HandCategory::FourOfAKind, {10, 14, 0, 0, 0}},
            {HandCategory::StraightFlush, {14, 0, 0, 0, 0}}
        };

        for (std::size_t i = 1; i < 9; ++i) {
            check(ranks[i] > ranks[i - 1],
                  "Categories must be ordered by poker strength");
        }
    }

    // TEST 9: Kicker comparison within the same category.
    {
        HandRank a(HandCategory::OnePair, {14, 13, 12, 9, 0});
        HandRank b(HandCategory::OnePair, {14, 13, 11, 10, 0});

        check(a > b, "Higher second kicker wins");

        HandRank c(HandCategory::TwoPair, {10, 8, 14, 0, 0});
        HandRank d(HandCategory::TwoPair, {10, 8, 13, 0, 0});

        check(c > d, "Higher TwoPair kicker wins");

        HandRank e(HandCategory::FullHouse, {10, 14, 0, 0, 0});
        HandRank f(HandCategory::FullHouse, {9, 14, 0, 0, 0});

        check(e > f, "Higher trips rank wins FullHouse comparison");
    }

    // TEST 10: Equal strength means a tie.
    {
        HandRank a(HandCategory::Flush, {14, 12, 9, 6, 3});
        HandRank b(HandCategory::Flush, {14, 12, 9, 6, 3});

        check(a == b, "Identical HandRank values must tie");
        check(!(a < b), "Equal ranks must not be less");
        check(!(a > b), "Equal ranks must not be greater");
        check(a <= b && a >= b,
              "Equal ranks must satisfy <= and >=");
    }

    // TEST 11: Constructor copies the input array.
    {
        HandRank::Kickers values{8, 14, 12, 10, 0};
        HandRank rank(HandCategory::OnePair, values);

        values[0] = 7;

        check(rank.kickers() == HandRank::Kickers{8, 14, 12, 10, 0},
              "Modifying the input must not change HandRank");
    }

    std::cout << "All HandRank tests passed.\n";
    return EXIT_SUCCESS;
}
