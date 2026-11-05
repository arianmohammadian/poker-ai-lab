/**
 * @file equity_calculator_test.cc
 * @brief Tests for exact Heads-Up Texas Hold'em equity calculation (flop, turn and river).
 * @author Arian Mohammadian
 */

#include "poker/equity_calculator.h"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

using poker::Card;
using poker::EquityCalculator;
using poker::EquityInput;
using poker::EquityResult;
using poker::Rank;
using poker::Suit;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

Card card(int rank, int suit) {
    return Card(static_cast<Rank>(rank), static_cast<Suit>(suit));
}

void checkResult(const EquityResult& result,
                 std::uint64_t expectedP1,
                 std::uint64_t expectedP2,
                 std::uint64_t expectedTies,
                 const char* message) {
    if (result.player1Wins != expectedP1 ||
        result.player2Wins != expectedP2 ||
        result.ties != expectedTies) {
        std::cerr << "FAILED: " << message
                  << " (got " << result.player1Wins << ", "
                  << result.player2Wins << ", " << result.ties << ")\n";
        std::exit(EXIT_FAILURE);
    }
}

void checkTotalAndEquity(const EquityResult& result,
                         std::uint64_t expectedTotal,
                         const char* message) {
    check(result.total() == expectedTotal, message);
    check(result.player1Equity() >= 0.0 && result.player1Equity() <= 1.0,
          "Player 1 equity is within [0, 1]");
    check(result.player2Equity() >= 0.0 && result.player2Equity() <= 1.0,
          "Player 2 equity is within [0, 1]");
    check(std::abs(result.player1Equity() + result.player2Equity() - 1.0) < 1e-12,
          "Heads-up equities sum to one");
}

void expectInvalid(const EquityInput& input, const char* message) {
    try {
        (void)EquityCalculator::calculateExact(input);
    } catch (const std::invalid_argument&) {
        return;
    }
    check(false, message);
}

void testRiver() {
    const std::vector<Card> board{
        card(2, 0), card(7, 1), card(9, 2), card(11, 3), card(4, 0)
    };

    const EquityInput p1Wins{{card(14, 0), card(14, 1)},
                             {card(13, 0), card(13, 1)}, board};
    const EquityInput p2Wins{{card(13, 0), card(13, 1)},
                             {card(14, 0), card(14, 1)}, board};
    const EquityInput tie{{card(3, 0), card(4, 1)},
                          {card(5, 0), card(6, 1)},
                          {card(10, 2), card(11, 2), card(12, 2),
                           card(13, 2), card(14, 2)}};

    checkResult(EquityCalculator::calculateExact(p1Wins), 1, 0, 0, "River P1 win");
    checkResult(EquityCalculator::calculateExact(p2Wins), 0, 1, 0, "River P2 win");
    const auto tied = EquityCalculator::calculateExact(tie);
    checkResult(tied, 0, 0, 1, "River tie");
    check(tied.player1Equity() == 0.5 && tied.player2Equity() == 0.5,
          "River tie splits equity equally");
}

void testTurnGuaranteedWinner() {
    // P1 has an unbeatable royal flush already on turn.
    const EquityInput input{
        {card(14, 3), card(13, 3)},
        {card(3, 1), card(4, 2)},
        {card(12, 3), card(11, 3), card(10, 3), card(2, 0)}
    };
    const auto result = EquityCalculator::calculateExact(input);
    checkResult(result, 44, 0, 0, "Turn unbeatable royal flush");
    checkTotalAndEquity(result, 44, "Turn has C(44,1) completions");
}

void testFlopCountAndSymmetry() {
    const EquityInput input{
        {card(14, 0), card(14, 1)},
        {card(13, 0), card(13, 1)},
        {card(2, 0), card(7, 1), card(9, 2)}
    };
    const auto first = EquityCalculator::calculateExact(input);
    const auto again = EquityCalculator::calculateExact(input);
    checkTotalAndEquity(first, 990, "Flop has C(45,2) completions");
    checkResult(again, first.player1Wins, first.player2Wins, first.ties,
                "Exact equity is deterministic");

    const EquityInput swapped{input.player2, input.player1, input.board};
    const auto reversed = EquityCalculator::calculateExact(swapped);
    checkTotalAndEquity(reversed, 990, "Swapped flop has 990 completions");
    checkResult(reversed, first.player2Wins, first.player1Wins, first.ties,
                "Swapping players swaps wins and preserves ties");
}

void testInvalidInput() {
    const EquityInput valid{
        {card(14, 0), card(14, 1)},
        {card(13, 0), card(13, 1)},
        {card(2, 0), card(7, 1), card(9, 2)}
    };

    for (std::size_t size : {std::size_t{1}, std::size_t{2}, std::size_t{6}}) {
        EquityInput wrongSize = valid;
        wrongSize.board = {
            card(2, 0), card(3, 0), card(4, 0),
            card(5, 0), card(6, 0), card(7, 0)
        };
        wrongSize.board.erase(wrongSize.board.begin() + static_cast<std::ptrdiff_t>(size),
                              wrongSize.board.end());
        expectInvalid(wrongSize, "Invalid board size must be rejected");
    }

    EquityInput withinPlayer = valid;
    withinPlayer.player1[1] = withinPlayer.player1[0];
    expectInvalid(withinPlayer, "Duplicate within a private hand");

    EquityInput betweenPlayers = valid;
    betweenPlayers.player2[0] = betweenPlayers.player1[0];
    expectInvalid(betweenPlayers, "Duplicate between players");

    EquityInput handAndBoard = valid;
    handAndBoard.board[0] = handAndBoard.player1[0];
    expectInvalid(handAndBoard, "Duplicate between a hand and the board");

    EquityInput withinBoard = valid;
    withinBoard.board[2] = withinBoard.board[0];
    expectInvalid(withinBoard, "Duplicate within the board");
}

} // namespace

int main() {
    testRiver();
    testTurnGuaranteedWinner();
    testFlopCountAndSymmetry();
    testInvalidInput();

    std::cout << "All exact equity tests passed (river, turn, flop, invalid input).\n";
    return EXIT_SUCCESS;
}
