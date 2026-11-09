/**
 * @file learning_agent_test.cc
 * @brief Tests tabular agent action selection and episode learning.
 * @author Arian Mohammadian
 */

#include "poker/learning_agent.h"
#include "poker/hand_engine.h"
#include "poker/preflop_equity_table.h"

#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>

using namespace poker;

namespace {

void check(bool ok, const char* message) {
    if (!ok) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void testSingleChoiceAndLearning() {
    std::mt19937 deckRng{55}, agentRng{56};
    HandEngine engine({1000, 1000}, {50, 100}, 0, deckRng);
    auto obs = engine.observe(0);

    // Restrict the action space to a single deterministic action.
    LegalActions onlyCheck{};
    onlyCheck.canCheck = true;
    obs.legalActions = onlyCheck;

    QTable table;
    LearningConfig config;
    config.epsilon = 0.0;
    config.equitySamples = 10;
    config.bigBlind = 100;
    LearningAgent agent(agentRng, table, config);

    const auto state = StateAbstractor::abstract(
        obs, PreflopEquityTable::lookup(obs.holeCards), config.bigBlind);

    const auto action = agent.chooseAction(obs);
    check(action.type == ActionType::Check, "Only available action must be check");
    check(table.visits(state, AbstractAction::CheckCall) == 0,
          "Choosing an action must not update Q before the reward");

    agent.finishEpisode(2.5);
    check(table.visits(state, AbstractAction::CheckCall) == 1,
          "Finishing an episode must update the experienced action");
    check(table.value(state, AbstractAction::CheckCall) == 2.5,
          "Q must equal the first received reward");

    agent.finishEpisode(50.0);
    check(table.visits(state, AbstractAction::CheckCall) == 1,
          "Finishing an empty episode must not learn again");

    [[maybe_unused]] const auto next = agent.chooseAction(obs);
    agent.clearEpisode();
    agent.finishEpisode(-10.0);
    check(table.visits(state, AbstractAction::CheckCall) == 1,
          "Cleared experiences must not update the table");
}

void testInvalidConfig() {
    std::mt19937 rng{4};
    QTable table;
    LearningConfig config;
    config.epsilon = 1.5;
    bool rejected = false;
    try { [[maybe_unused]] LearningAgent agent(rng, table, config); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Invalid epsilon must be rejected");
}

} // namespace

int main() {
    testSingleChoiceAndLearning();
    testInvalidConfig();
    std::cout << "All LearningAgent tests passed!\n";
    return EXIT_SUCCESS;
}
