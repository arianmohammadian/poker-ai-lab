
/**
 * @file self_play_runner_test.cc
 * @brief Tests single-hand self-play execution and rewards.
 * @author Arian Mohammadian
 */

#include "poker/self_play_runner.h"
#include "poker/random_agent.h"

#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>

using namespace poker;

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

class FoldAgent final : public Agent {
public:
    Action chooseAction(const PlayerObservation& obs) override {
        if (!obs.legalActions)
            throw std::logic_error("Missing legal actions");

        if (obs.legalActions->canFold)
            return {ActionType::Fold, 0};

        return {ActionType::Check, 0};
    }
};

void testFoldAndRewards() {
    FoldAgent agent0;
    FoldAgent agent1;

    const auto result = SelfPlayRunner::runHand(
        agent0, agent1, {1000, 1000}, {50, 100}, 0, 42
    );

    check(result.decisions == 1,
          "Preflop fold must end the hand in one decision");

    check(result.finalStacks[0] == 950 &&
          result.finalStacks[1] == 1050,
          "Incorrect stacks after button folds");

    check(result.rewards[0] == -50 &&
          result.rewards[1] == 50,
          "Rewards must equal net chip changes");

    check(result.rewards[0] + result.rewards[1] == 0,
          "Self-play must conserve chips");
}

void testOtherButton() {
    FoldAgent agent0;
    FoldAgent agent1;

    const auto result = SelfPlayRunner::runHand(
        agent0, agent1, {1000, 1000}, {50, 100}, 1, 43
    );

    check(result.finalStacks[0] == 1050 &&
          result.finalStacks[1] == 950,
          "Incorrect outcome with button at seat 1");

    check(result.rewards[0] == 50 &&
          result.rewards[1] == -50,
          "Rewards must reflect the opposite winner");
}

void testReproducibility() {
    std::mt19937 rngA1{101}, rngB1{102};
    std::mt19937 rngA2{101}, rngB2{102};

    RandomAgent a1(rngA1), b1(rngB1);
    RandomAgent a2(rngA2), b2(rngB2);

    const auto first = SelfPlayRunner::runHand(
        a1, b1, {1000, 1000}, {50, 100}, 0, 99
    );

    const auto second = SelfPlayRunner::runHand(
        a2, b2, {1000, 1000}, {50, 100}, 0, 99
    );

    check(first.finalStacks == second.finalStacks,
          "Identical seeds must produce identical stacks");
    check(first.rewards == second.rewards,
          "Identical seeds must produce identical rewards");
    check(first.decisions == second.decisions,
          "Identical seeds must produce identical action counts");

    check(first.rewards[0] + first.rewards[1] == 0,
          "Total reward must be zero");
}

} // namespace

int main() {
    testFoldAndRewards();
    testOtherButton();
    testReproducibility();

    std::cout << "All SelfPlayRunner tests passed!\n";
    return EXIT_SUCCESS;
}
