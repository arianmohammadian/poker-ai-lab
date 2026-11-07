
/**
 * @file equity_unknown_test.cc
 * @brief Tests for Monte Carlo equity against an unknown opponent hand.
 * @author Arian Mohammadian
 */

#include "poker/equity_calculator.h"
#include "poker/hand_evaluator.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

using poker::Card;
using poker::EquityCalculator;
using poker::EquityResult;
using poker::HandEvaluator;
using poker::Rank;
using poker::Suit;

Card card(int rank, int suit) {
    return Card(static_cast<Rank>(rank), static_cast<Suit>(suit));
}

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void checkResult(
    const EquityResult& result,
    std::uint64_t wins,
    std::uint64_t losses,
    std::uint64_t ties,
    const char* message
) {
    if (result.player1Wins != wins ||
        result.player2Wins != losses ||
        result.ties != ties) {
        std::cerr << "FAILED: " << message
                  << " (got " << result.player1Wins
                  << ", " << result.player2Wins
                  << ", " << result.ties << ")\n";
        std::exit(EXIT_FAILURE);
    }
}

void checkTotalAndEquity(
    const EquityResult& result,
    std::uint64_t samples
) {
    check(result.total() == samples,
          "Total must equal sample count");

    check(result.player1Equity() >= 0.0 &&
          result.player1Equity() <= 1.0,
          "Hero equity must be within [0, 1]");

    check(result.player2Equity() >= 0.0 &&
          result.player2Equity() <= 1.0,
          "Opponent equity must be within [0, 1]");

    check(std::abs(
        result.player1Equity() +
        result.player2Equity() - 1.0
    ) < 1e-12, "Equities must sum to one");
}

void expectInvalid(
    const std::array<Card, 2>& hero,
    const std::vector<Card>& board,
    std::uint64_t samples,
    const char* message
) {
    try {
        (void)EquityCalculator::calculateMonteCarloVsUnknown(
            hero, board, samples, 42
        );
    } catch (const std::invalid_argument&) {
        return;
    }

    check(false, message);
}

// TEST 1: Invalid inputs.
void testInvalidInput() {
    const std::array<Card, 2> hero{
        card(14, 3), card(14, 2)
    };

    expectInvalid(hero, {}, 0,
                  "Zero samples must be rejected");

    for (std::size_t size : {1u, 2u, 6u}) {
        std::vector<Card> board{
            card(2, 0), card(3, 0), card(4, 0),
            card(5, 0), card(6, 0), card(7, 0)
        };

        board.erase(board.begin() + size, board.end());

        expectInvalid(hero, board, 100,
                      "Invalid board size must be rejected");
    }

    const std::array<Card, 2> duplicateHero{
        card(14, 3), card(14, 3)
    };

    expectInvalid(duplicateHero, {}, 100,
                  "Duplicate hero cards must be rejected");

    expectInvalid(
        hero,
        {card(14, 3), card(5, 0), card(7, 1)},
        100,
        "Duplicate between hero and board"
    );

    expectInvalid(
        hero,
        {card(2, 0), card(2, 0), card(7, 1)},
        100,
        "Duplicate within board"
    );
}

// TEST 2: Sample count and normalized equity.
void testSampleCount() {
    const std::array<Card, 2> hero{
        card(14, 3), card(13, 3)
    };

    const auto result =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, {}, 1000, 11
        );

    checkTotalAndEquity(result, 1000);
}

// TEST 3: Determinism with a fixed seed.
void testReproducibility() {
    const std::array<Card, 2> hero{
        card(14, 3), card(12, 3)
    };

    const std::vector<Card> board{
        card(13, 3), card(11, 2), card(4, 1)
    };

    const auto first =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, board, 1000, 12345
        );

    const auto second =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, board, 1000, 12345
        );

    checkResult(
        second,
        first.player1Wins,
        first.player2Wins,
        first.ties,
        "Same seed must produce identical results"
    );
}

// TEST 4: Preflop pocket aces against a random hand.
void testPreflopAces() {
    const std::array<Card, 2> hero{
        card(14, 3), card(14, 2)
    };

    constexpr std::uint64_t samples = 10000;

    const auto result =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, {}, samples, 2026
        );

    checkTotalAndEquity(result, samples);

    // Pocket aces have approximately 85% equity
    // against one uniformly random opponent hand.
    check(
        std::abs(result.player1Equity() - 0.85) < 0.05,
        "Preflop AA equity should be close to 85%"
    );
}

// TEST 5: Flop and turn simulations.
void testPartialBoards() {
    const std::array<Card, 2> hero{
        card(14, 3), card(12, 3)
    };

    const std::vector<Card> flop{
        card(13, 3), card(11, 3), card(4, 1)
    };

    const std::vector<Card> turn{
        card(13, 3), card(11, 3),
        card(4, 1), card(2, 0)
    };

    const auto flopResult =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, flop, 2000, 42
        );

    const auto turnResult =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, turn, 2000, 42
        );

    checkTotalAndEquity(flopResult, 2000);
    checkTotalAndEquity(turnResult, 2000);
}

// TEST 6: Outcomes that are mathematically certain.
void testGuaranteedOutcomes() {
    // Hero holds an unbeatable royal flush.
    const std::array<Card, 2> hero{
        card(14, 3), card(13, 3)
    };

    const std::vector<Card> winningBoard{
        card(12, 3), card(11, 3), card(10, 3),
        card(2, 0), card(3, 1)
    };

    constexpr std::uint64_t samples = 1000;

    const auto win =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, winningBoard, samples, 7
        );

    checkResult(win, samples, 0, 0,
                "Unbeatable royal flush must always win");

    // The board itself is a royal flush.
    // Both players necessarily play the board.
    const std::array<Card, 2> otherHero{
        card(2, 0), card(3, 1)
    };

    const std::vector<Card> tiedBoard{
        card(10, 3), card(11, 3), card(12, 3),
        card(13, 3), card(14, 3)
    };

    const auto tie =
        EquityCalculator::calculateMonteCarloVsUnknown(
            otherHero, tiedBoard, samples, 7
        );

    checkResult(tie, 0, 0, samples,
                "Royal flush on board must always tie");

    check(tie.player1Equity() == 0.5,
          "Guaranteed tie must give 50% equity");
}

// Independently enumerate all opponent hands on the river.
EquityResult exactRiverReference(
    const std::array<Card, 2>& hero,
    const std::vector<Card>& board
) {
    std::array<bool, 52> used{};

    auto mark = [&](const Card& c) {
        const int id =
            (static_cast<int>(c.rank()) - 2) * 4 +
            static_cast<int>(c.suit());

        used[id] = true;
    };

    for (const Card& c : hero) mark(c);
    for (const Card& c : board) mark(c);

    std::vector<Card> remaining;
    remaining.reserve(45);

    for (int rank = 2; rank <= 14; ++rank) {
        for (int suit = 0; suit < 4; ++suit) {
            const int id = (rank - 2) * 4 + suit;

            if (!used[id])
                remaining.push_back(card(rank, suit));
        }
    }

    const std::array<Card, 7> heroCards{
        hero[0], hero[1],
        board[0], board[1], board[2],
        board[3], board[4]
    };

    const auto heroRank = HandEvaluator::evaluate(heroCards);

    EquityResult result;

    for (std::size_t i = 0; i < remaining.size(); ++i) {
        for (std::size_t j = i + 1;
             j < remaining.size(); ++j) {

            const std::array<Card, 7> opponentCards{
                remaining[i], remaining[j],
                board[0], board[1], board[2],
                board[3], board[4]
            };

            const auto opponentRank =
                HandEvaluator::evaluate(opponentCards);

            if (heroRank > opponentRank)
                ++result.player1Wins;
            else if (heroRank < opponentRank)
                ++result.player2Wins;
            else
                ++result.ties;
        }
    }

    return result;
}

// TEST 7: Monte Carlo versus exact river enumeration.
void testRiverAgainstExact() {
    const std::array<Card, 2> hero{
        card(14, 0), card(14, 1)
    };

    const std::vector<Card> board{
        card(2, 0), card(7, 1), card(9, 2),
        card(11, 3), card(4, 0)
    };

    const auto exact = exactRiverReference(hero, board);

    check(exact.total() == 990,
          "River must have exactly 990 opponent hands");

    constexpr std::uint64_t samples = 20000;

    const auto mc =
        EquityCalculator::calculateMonteCarloVsUnknown(
            hero, board, samples, 2026
        );

    checkTotalAndEquity(mc, samples);

    const double difference =
        std::abs(mc.player1Equity() -
                 exact.player1Equity());

    check(difference < 0.04,
          "River Monte Carlo must approximate exact equity");
}

} // namespace

int main() {
    testInvalidInput();
    testSampleCount();
    testReproducibility();
    testPreflopAces();
    testPartialBoards();
    testGuaranteedOutcomes();
    testRiverAgainstExact();

    std::cout
        << "All unknown opponent equity tests passed "
        << "(validation, samples, reproducibility, "
        << "preflop, flop, turn, guaranteed outcomes, "
        << "river exact reference).\n";

    return EXIT_SUCCESS;
}
