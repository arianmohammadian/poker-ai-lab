/**
 * @file equity_result_test.cc
 * @brief Unit tests for Heads-Up equity result counters and probabilities.
 * @author Arian Mohammadian
 */

#include "poker/equity_result.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

using poker::EquityResult;

constexpr double kTolerance = 1e-12;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void checkNear(double actual, double expected, const char* message) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= kTolerance,
          message);
}

void expectLogicErrorForEmptyResult() {
    const EquityResult empty{};
    check(empty.total() == 0, "Default result has zero scenarios");

    bool player1Throws = false;
    try {
        (void)empty.player1Equity();
    } catch (const std::logic_error&) {
        player1Throws = true;
    }
    check(player1Throws, "Player 1 equity rejects an empty result");

    bool player2Throws = false;
    try {
        (void)empty.player2Equity();
    } catch (const std::logic_error&) {
        player2Throws = true;
    }
    check(player2Throws, "Player 2 equity rejects an empty result");
}

void checkResult(const EquityResult& result,
                 std::uint64_t expectedTotal,
                 double expectedPlayer1Equity,
                 double expectedPlayer2Equity,
                 const char* totalMessage,
                 const char* player1Message,
                 const char* player2Message) {
    check(result.total() == expectedTotal, totalMessage);
    checkNear(result.player1Equity(), expectedPlayer1Equity, player1Message);
    checkNear(result.player2Equity(), expectedPlayer2Equity, player2Message);
    checkNear(result.player1Equity() + result.player2Equity(), 1.0,
              "Heads-Up equities sum to one");
}

void testResults() {
    checkResult(EquityResult{10, 0, 0}, 10, 1.0, 0.0,
                "All-player1 total", "All-player1 equity", "All-player1 opponent equity");
    checkResult(EquityResult{0, 10, 0}, 10, 0.0, 1.0,
                "All-player2 total", "All-player2 equity", "All-player2 opponent equity");
    checkResult(EquityResult{0, 0, 10}, 10, 0.5, 0.5,
                "All-ties total", "All-ties player1 equity", "All-ties player2 equity");
    checkResult(EquityResult{3, 5, 2}, 10, 0.4, 0.6,
                "Mixed total", "Mixed player1 equity", "Mixed player2 equity");
    checkResult(EquityResult{1, 0, 1}, 2, 0.75, 0.25,
                "Odd-tie total", "Odd-tie player1 equity", "Odd-tie player2 equity");
    checkResult(EquityResult{1, 1, 1}, 3, 0.5, 0.5,
                "Three-scenario total", "Three-scenario player1 equity", "Three-scenario player2 equity");
}

} // namespace

int main() {
    testResults();
    expectLogicErrorForEmptyResult();
    std::cout << "All EquityResult tests passed.\n";
    return EXIT_SUCCESS;
}
