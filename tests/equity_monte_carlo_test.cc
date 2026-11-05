/**
 * @file equity_monte_carlo_test.cc
 * @brief Deterministic, validation and statistical tests for Monte Carlo equity.
 * @author Arian Mohammadian
 */

#include "poker/equity_calculator.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

using poker::Card;
using poker::EquityCalculator;
using poker::EquityInput;
using poker::EquityResult;
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

void expectInvalid(const EquityInput& input, std::uint64_t samples,
                   const char* message) {
    try {
        (void)EquityCalculator::calculateMonteCarlo(input, samples, 42);
    } catch (const std::invalid_argument&) {
        return;
    }
    check(false, message);
}

void testRiver() {
    // AA beats KK on this complete board, so every sample must be a P1 win.
    const EquityInput input{
        {card(14, 2), card(14, 3)},
        {card(13, 2), card(13, 3)},
        {card(2, 0), card(7, 1), card(11, 3), card(4, 0), card(9, 2)}
    };

    for (const std::uint64_t samples : {std::uint64_t{1}, std::uint64_t{37}}) {
        const EquityResult result = EquityCalculator::calculateMonteCarlo(input, samples, 7);
        check(result.total() == samples, "River: total must equal samples");
        check(result.player1Wins == samples, "River: P1 must win every sample");
        check(result.player2Wins == 0 && result.ties == 0, "River: no losses or ties");
        check(result.player1Equity() == 1.0, "River: P1 equity must be 1");
    }
}

void testRiverTie() {
    // A royal flush on the board means every showdown ties.
    const EquityInput input{
        {card(2, 0), card(3, 1)},
        {card(4, 0), card(5, 1)},
        {card(10, 2), card(11, 2), card(12, 2), card(13, 2), card(14, 2)}
    };

    const EquityResult result = EquityCalculator::calculateMonteCarlo(input, 51, 9);
    check(result.total() == 51, "River tie: total must equal samples");
    check(result.ties == 51, "River tie: all samples must tie");
    check(result.player1Wins == 0 && result.player2Wins == 0, "River tie: no wins");
    check(result.player1Equity() == 0.5 && result.player2Equity() == 0.5,
          "River tie: equity must be split equally");
}

void testTurnGuaranteedWinner() {
    // P1 already holds an unbeatable royal flush, regardless of the river.
    const EquityInput input{
        {card(14, 2), card(13, 2)},
        {card(2, 0), card(3, 1)},
        {card(12, 2), card(11, 2), card(10, 2), card(7, 0)}
    };

    const EquityResult result = EquityCalculator::calculateMonteCarlo(input, 200, 1234);
    check(result.total() == 200, "Turn: total must equal samples");
    check(result.player1Wins == 200, "Turn: unbeatable hand must always win");
    check(result.player2Wins == 0 && result.ties == 0, "Turn: no losses or ties");
}

void testReproducibility() {
    const EquityInput input{
        {card(14, 2), card(14, 3)},
        {card(13, 2), card(13, 3)},
        {} // Preflop
    };

    constexpr std::uint64_t samples = 1000;
    const EquityResult first = EquityCalculator::calculateMonteCarlo(input, samples, 2026);
    const EquityResult second = EquityCalculator::calculateMonteCarlo(input, samples, 2026);

    check(first.total() == samples, "Reproducibility: correct sample count");
    check(first.player1Wins == second.player1Wins, "Reproducibility: same P1 wins");
    check(first.player2Wins == second.player2Wins, "Reproducibility: same P2 wins");
    check(first.ties == second.ties, "Reproducibility: same ties");
}

void testConvergenceOnFlop() {
    // Compare with an independently implemented exhaustive mode on just 990 boards.
    const EquityInput input{
        {card(14, 2), card(14, 3)},
        {card(13, 2), card(13, 3)},
        {card(2, 0), card(7, 1), card(11, 3)}
    };

    const EquityResult exact = EquityCalculator::calculateExact(input);
    constexpr std::uint64_t samples = 30000;
    const EquityResult approximate = EquityCalculator::calculateMonteCarlo(input, samples, 8675309);

    check(exact.total() == 990, "Flop: exact method must enumerate 990 outcomes");
    check(approximate.total() == samples, "Flop: MC total must equal samples");
    check(std::abs(approximate.player1Equity() - exact.player1Equity()) < 0.03,
          "Flop: MC equity must be close to exact equity");
    check(std::abs(approximate.player1Equity() + approximate.player2Equity() - 1.0) < 1e-12,
          "Flop: both equities must sum to 1");
}

void testInvalidInputs() {
    const EquityInput valid{
        {card(14, 2), card(14, 3)},
        {card(13, 2), card(13, 3)},
        {card(2, 0), card(7, 1), card(11, 3)}
    };

    expectInvalid(valid, 0, "Zero samples must throw invalid_argument");

    const EquityInput duplicate{
        {card(14, 2), card(14, 3)},
        {card(13, 2), card(13, 3)},
        {card(14, 2), card(7, 1), card(11, 3)}
    };
    expectInvalid(duplicate, 20, "Duplicate card must throw invalid_argument");

    const EquityInput invalidBoard{
        {card(14, 2), card(14, 3)},
        {card(13, 2), card(13, 3)},
        {card(2, 0), card(7, 1)}
    };
    expectInvalid(invalidBoard, 20, "Two-card board must throw invalid_argument");
}

} // namespace

int main() {
    testRiver();
    testRiverTie();
    testTurnGuaranteedWinner();
    testReproducibility();
    testConvergenceOnFlop();
    testInvalidInputs();

    std::cout << "All Monte Carlo equity tests passed.\n";
    return EXIT_SUCCESS;
}
