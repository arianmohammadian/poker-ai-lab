/**
 * @file abstract_action_test.cc
 * @brief Tests legal abstract actions and concrete betting conversions.
 * @author Arian Mohammadian
 */

#include "poker/abstract_action.h"
#include "poker/hand_engine.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using namespace poker;

namespace {

void check(bool ok, const char* message) {
    if (!ok) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

bool contains(const std::vector<AbstractAction>& actions, AbstractAction action) {
    return std::find(actions.begin(), actions.end(), action) != actions.end();
}

void testPreflopRaise() {
    std::mt19937 rng{101};
    HandEngine engine({1000, 1000}, {50, 100}, 0, rng);
    const auto obs = engine.observe(0);

    const auto half = ActionAbstractor::toConcrete(AbstractAction::HalfPot, obs);
    const auto pot = ActionAbstractor::toConcrete(AbstractAction::Pot, obs);
    const auto allIn = ActionAbstractor::toConcrete(AbstractAction::AllIn, obs);

    check(half.type == ActionType::Raise && half.amount == 200,
          "Half-pot preflop raise should clamp to minimum 200");
    check(pot.type == ActionType::Raise && pot.amount == 300,
          "Pot-sized preflop raise should be 300 total");
    check(allIn.type == ActionType::Raise && allIn.amount == 1000,
          "Preflop all-in must use total street commitment");

    auto actions = ActionAbstractor::legalActions(obs);
    check(contains(actions, AbstractAction::CheckCall), "Call must be available");
    check(contains(actions, AbstractAction::AllIn), "All-in must be available");

    // Conversion must result in moves accepted by the engine.
    engine.applyAction(0, half);
    check(engine.state().actingPlayer() == 1, "Opponent must respond to raise");
}

void testUnopenedFlopAndDuplicates() {
    std::mt19937 rng{102};
    HandEngine engine({1000, 1000}, {50, 100}, 0, rng);
    engine.applyAction(0, {ActionType::Call, 0});
    engine.applyAction(1, {ActionType::Check, 0});
    const auto obs = engine.observe(1);

    const auto half = ActionAbstractor::toConcrete(AbstractAction::HalfPot, obs);
    const auto pot = ActionAbstractor::toConcrete(AbstractAction::Pot, obs);
    const auto allIn = ActionAbstractor::toConcrete(AbstractAction::AllIn, obs);
    check(half.type == ActionType::Bet && half.amount == 100,
          "Half pot must clamp to minimum bet");
    check(pot.type == ActionType::Bet && pot.amount == 200,
          "Pot must bet current pot");
    check(allIn.type == ActionType::Bet && allIn.amount == 900,
          "All-in bet must use remaining street stack");

    const auto legal = ActionAbstractor::legalActions(obs);
    check(contains(legal, AbstractAction::CheckCall), "Check must be available");
    // The current engine permits folding even when checking is free.
    check(contains(legal, AbstractAction::Fold), "Fold is currently legal on the flop");

    // Force half-pot and pot to map to the same minimum bet.
    auto duplicate = obs;
    duplicate.pot = 20;
    const auto candidates = ActionAbstractor::legalActions(duplicate);
    check(contains(candidates, AbstractAction::HalfPot), "Half-pot must be retained");
    check(!contains(candidates, AbstractAction::Pot), "Duplicate concrete bet must be removed");
}

void testShortAllInCall() {
    std::mt19937 rng{103};
    HandEngine engine({75, 1000}, {50, 100}, 0, rng);
    const auto obs = engine.observe(0);

    check(!obs.legalActions->canRaise, "Short stack cannot raise over big blind");
    const auto action = ActionAbstractor::toConcrete(AbstractAction::AllIn, obs);
    check(action.type == ActionType::Call && action.amount == 0,
          "Call with insufficient stack must be all-in");

    const auto actions = ActionAbstractor::legalActions(obs);
    check(contains(actions, AbstractAction::CheckCall), "Short call must be legal");
    check(!contains(actions, AbstractAction::AllIn),
          "Same short all-in call must not appear twice");
}

void testInvalidObservation() {
    std::mt19937 rng{104};
    HandEngine engine({1000, 1000}, {50, 100}, 0, rng);
    auto obs = engine.observe(0);
    obs.legalActions.reset();
    bool rejected = false;
    try {
        [[maybe_unused]] auto actions = ActionAbstractor::legalActions(obs);
    } catch (const std::logic_error&) {
        rejected = true;
    }
    check(rejected, "Missing legal actions must be rejected");
}

} // namespace

int main() {
    testPreflopRaise();
    testUnopenedFlopAndDuplicates();
    testShortAllInCall();
    testInvalidObservation();
    std::cout << "All AbstractAction tests passed!\n";
    return EXIT_SUCCESS;
}
