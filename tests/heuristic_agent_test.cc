/**
 * @file heuristic_agent_test.cc
 * @brief Tests for the first passive equity-based HeuristicAgent.
 * @author Arian Mohammadian
 */

#include "poker/heuristic_agent.h"
#include "poker/random_agent.h"
#include "poker/hand_engine.h"
#include "poker/hand_phase.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using poker::Action;
using poker::ActionType;
using poker::Card;
using poker::HandConfig;
using poker::HandEngine;
using poker::HandPhase;
using poker::HeuristicAgent;
using poker::PlayerObservation;
using poker::RandomAgent;
using poker::Rank;
using poker::Suit;

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

Card card(int rank, int suit) {
    return Card(static_cast<Rank>(rank), static_cast<Suit>(suit));
}

void checkAction(Action action, ActionType expected, const char* message) {
    check(action.type == expected, message);
    check(action.amount == 0, "Passive action amount must be zero");
}

// Return a real observation for the small blind facing the big blind.
PlayerObservation initialObservation(std::uint32_t seed = 42) {
    std::mt19937 deckRng(seed);
    HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, deckRng};
    return engine.observe(0);
}

// A guaranteed split pot: the royal flush is entirely on the board.
// Useful for checking pot-odds thresholds without Monte Carlo noise.
void setGuaranteedTie(PlayerObservation& observation) {
    observation.holeCards = {card(2, 0), card(3, 1)};
    observation.board = {
        card(10, 3), card(11, 3), card(12, 3),
        card(13, 3), card(14, 3)
    };
    check(observation.legalActions.has_value(),
          "Expected an observation with legal actions");
    observation.legalActions->canCheck = false;
    observation.legalActions->canCall = true;
}

// Test 1: No actions are available for a player who is not acting.
void testNoLegalActions() {
    std::mt19937 deckRng{42};
    std::mt19937 agentRng{100};
    HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, deckRng};
    HeuristicAgent agent(agentRng);

    const PlayerObservation observation = engine.observe(1);
    check(!observation.legalActions.has_value(),
          "Waiting player must not receive legal actions");

    bool threw = false;
    try {
        (void)agent.chooseAction(observation);
    } catch (const std::logic_error&) {
        threw = true;
    }
    check(threw, "Agent must reject missing legal actions");
}

// Test 2: The agent must never fold when it can check for free.
void testCheck() {
    std::mt19937 deckRng{43};
    std::mt19937 agentRng{101};
    HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, deckRng};
    HeuristicAgent agent(agentRng);

    engine.applyAction(0, Action{ActionType::Call, 0});
    const PlayerObservation observation = engine.observe(1);
    check(observation.legalActions.has_value() &&
              observation.legalActions->canCheck,
          "Big blind must be allowed to check after a preflop call");

    const auto before = agentRng;
    checkAction(agent.chooseAction(observation), ActionType::Check,
                "Agent must check when checking is legal");
    check(agentRng == before,
          "Checking must not consume Monte Carlo randomness");
}

// Test 3: Call when the hero is guaranteed to win at showdown.
void testGuaranteedCall() {
    std::mt19937 agentRng{102};
    HeuristicAgent agent(agentRng);
    auto observation = initialObservation(44);

    observation.holeCards = {card(14, 3), card(13, 3)};
    observation.board = {
        card(12, 3), card(11, 3), card(10, 3),
        card(2, 0), card(3, 1)
    };

    check(observation.legalActions->canCall,
          "Small blind must have a legal preflop call");
    checkAction(agent.chooseAction(observation), ActionType::Call,
                "Unbeatable hero must call a normal bet");
}

// Test 4: Fold with 50% equity if the price requires 75%.
void testFoldWhenPotOddsAreBad() {
    std::mt19937 agentRng{103};
    HeuristicAgent agent(agentRng);
    auto observation = initialObservation(45);
    setGuaranteedTie(observation);

    observation.pot = 100;
    observation.legalActions->toCall = 300;
    // Required equity = 300 / (100 + 300) = 75%.
    checkAction(agent.chooseAction(observation), ActionType::Fold,
                "Agent must fold when 50% equity is below 75% pot odds");
}

// Test 5: Call with 50% equity if the price requires 25%.
void testCallWhenPotOddsAreGood() {
    std::mt19937 agentRng{104};
    HeuristicAgent agent(agentRng);
    auto observation = initialObservation(46);
    setGuaranteedTie(observation);

    observation.pot = 300;
    observation.legalActions->toCall = 100;
    // Required equity = 100 / (300 + 100) = 25%.
    checkAction(agent.chooseAction(observation), ActionType::Call,
                "Agent must call when 50% equity exceeds 25% pot odds");
}

// Test 6: No call or check available: the agent must fold without MC.
void testNoCallAvailable() {
    std::mt19937 agentRng{106};
    HeuristicAgent agent(agentRng);
    auto observation = initialObservation(49);
    observation.legalActions->canCheck = false;
    observation.legalActions->canCall = false;
    observation.legalActions->canFold = true;

    const auto before = agentRng;
    checkAction(agent.chooseAction(observation), ActionType::Fold,
                "Agent must fold when neither check nor call is legal");
    check(agentRng == before,
          "No-call path must not consume Monte Carlo randomness");
}

// Test 7: Same seed and observation produce the same action sequence.
void testReproducibility() {
    auto observation = initialObservation(47);
    std::mt19937 rng1{12345};
    std::mt19937 rng2{12345};
    HeuristicAgent agent1(rng1);
    HeuristicAgent agent2(rng2);

    for (int i = 0; i < 20; ++i) {
        const Action a = agent1.chooseAction(observation);
        const Action b = agent2.chooseAction(observation);
        check(a.type == b.type && a.amount == b.amount,
              "Equal RNG seeds must produce equal decisions");
    }
    check(rng1 == rng2,
          "Equal decision sequences must keep RNGs synchronized");
}

// Test 8: A legal Call with a short stack must be executed as all-in.
void testShortAllInCall() {
    std::mt19937 deckRng{48};
    std::mt19937 agentRng{105};
    HandEngine engine{{75, 1000}, HandConfig{50, 100}, 0, deckRng};
    HeuristicAgent agent(agentRng);

    auto observation = engine.observe(0);
    check(observation.legalActions.has_value() &&
              observation.legalActions->canCall,
          "Short-stacked small blind must be allowed to call");
    check(observation.stacks[0] < observation.legalActions->toCall,
          "Fixture must represent a short all-in call");

    // Force an unbeatable hero in the observation to test the agent's
    // action selection; HandEngine retains the real cards internally.
    observation.holeCards = {card(14, 3), card(13, 3)};
    observation.board = {
        card(12, 3), card(11, 3), card(10, 3),
        card(2, 0), card(3, 1)
    };

    const Action action = agent.chooseAction(observation);
    checkAction(action, ActionType::Call,
                "Agent must select Call even with insufficient chips");
    engine.applyAction(0, action);
    check(engine.state().phase() == HandPhase::Finished,
          "A short all-in call must resolve the hand");
    check(engine.state().player(0).stack() +
              engine.state().player(1).stack() == 1075,
          "Chips must be conserved after a short all-in");
}

// Test 9: HeuristicAgent vs RandomAgent must complete real hands.
void testAutonomousHands() {
    constexpr int kHands = 40;
    std::mt19937 deckRng{500};
    std::mt19937 heuristicRng{501};
    std::mt19937 randomRng{502};
    HeuristicAgent heuristic(heuristicRng);
    RandomAgent random(randomRng);

    for (int hand = 0; hand < kHands; ++hand) {
        HandEngine engine{
            {1000, 1000}, HandConfig{50, 100},
            static_cast<std::uint8_t>(hand % 2), deckRng
        };

        int actions = 0;
        while (engine.state().phase() == HandPhase::Betting) {
            check(actions++ < 100,
                  "Autonomous hand must not loop indefinitely");
            const auto actor = engine.state().actingPlayer();
            check(actor.has_value(),
                  "Betting phase requires an acting player");

            const auto player = *actor;
            const auto observation = engine.observe(player);
            check(observation.legalActions.has_value(),
                  "Acting player must have legal actions");

            // Alternate which seat holds the heuristic agent.
            const bool heuristicTurn = (player == hand % 2);
            const Action action = heuristicTurn
                ? heuristic.chooseAction(observation)
                : random.chooseAction(observation);

            // Throws if the chosen action is illegal.
            engine.applyAction(player, action);
        }

        check(engine.state().phase() == HandPhase::Finished,
              "Every autonomous hand must finish");
        check(engine.state().player(0).stack() +
                  engine.state().player(1).stack() == 2000,
              "Autonomous hand must conserve chips");
    }
}

} // namespace

int main() {
    testNoLegalActions();
    testCheck();
    testGuaranteedCall();
    testFoldWhenPotOddsAreBad();
    testCallWhenPotOddsAreGood();
    testNoCallAvailable();
    testReproducibility();
    testShortAllInCall();
    testAutonomousHands();

    std::cout << "All HeuristicAgent tests passed!\n";
    return EXIT_SUCCESS;
}
