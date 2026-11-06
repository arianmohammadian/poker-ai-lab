
/**
 * @file random_agent_test.cc
 * @brief Tests random poker agents and autonomous hand execution.
 * @author Arian Mohammadian
 */

#include "poker/random_agent.h"
#include "poker/hand_engine.h"
#include "poker/player_observation.h"
#include "poker/action.h"
#include "poker/hand_phase.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>

using poker::Action;
using poker::ActionType;
using poker::HandConfig;
using poker::HandEngine;
using poker::HandPhase;
using poker::PlayerObservation;
using poker::RandomAgent;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main() {

    // Test 1: agent rejects observations without legal actions
    {
        std::mt19937 deckRng{42};
        std::mt19937 agentRng{100};

        HandEngine engine{
            {1000, 1000}, HandConfig{50, 100}, 0, deckRng
        };

        RandomAgent agent{agentRng};

        PlayerObservation obs = engine.observe(1);

        check(!obs.legalActions.has_value(),
              "Waiting player must not have legal actions");

        bool exceptionThrown = false;

        try {
            [[maybe_unused]] Action action = agent.chooseAction(obs);
        }
        catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        check(exceptionThrown,
              "Agent must reject observations without legal actions");
    }

    // Test 2: initial preflop actions must be legal
    {
        std::mt19937 deckRng{43};
        std::mt19937 agentRng{101};

        HandEngine engine{
            {1000, 1000}, HandConfig{50, 100}, 0, deckRng
        };

        RandomAgent agent{agentRng};

        PlayerObservation obs = engine.observe(0);

        for (int i = 0; i < 1000; ++i) {
            Action action = agent.chooseAction(obs);

            check(
                action.type == ActionType::Fold ||
                action.type == ActionType::Call ||
                action.type == ActionType::Raise,
                "Preflop agent must select an available action"
            );

            if (action.type == ActionType::Fold ||
                action.type == ActionType::Call) {
                check(action.amount == 0,
                      "Passive actions must have amount zero");
            }

            if (action.type == ActionType::Raise) {
                check(action.amount >= 200 &&
                      action.amount <= 1000,
                      "Raise amount must be within legal bounds");
            }
        }
    }

    // Test 3: same seeds must produce identical decisions
    {
        std::mt19937 deckRng{44};
        std::mt19937 rng1{12345};
        std::mt19937 rng2{12345};

        HandEngine engine{
            {1000, 1000}, HandConfig{50, 100}, 0, deckRng
        };

        RandomAgent agent1{rng1};
        RandomAgent agent2{rng2};

        PlayerObservation obs = engine.observe(0);

        for (int i = 0; i < 1000; ++i) {
            Action a1 = agent1.chooseAction(obs);
            Action a2 = agent2.chooseAction(obs);

            check(a1.type == a2.type,
                  "Same seed must produce same action type");

            check(a1.amount == a2.amount,
                  "Same seed must produce same action amount");
        }
    }

    // Test 4: short all-in raise must be handled
    {
        std::mt19937 deckRng{45};
        std::mt19937 agentRng{102};

        HandEngine engine{
            {175, 1000}, HandConfig{50, 100}, 0, deckRng
        };

        RandomAgent agent{agentRng};
        PlayerObservation obs = engine.observe(0);

        check(obs.legalActions->shortAllInRaise.has_value(),
              "Short all-in raise must be available");

        // Force the agent to choose Raise.
        obs.legalActions->canFold = false;
        obs.legalActions->canCall = false;

        Action action = agent.chooseAction(obs);

        check(action.type == ActionType::Raise,
              "Agent must select the only available action");

        check(action.amount == 175,
              "Agent must select the correct all-in amount");

        engine.applyAction(0, action);

        check(engine.state().player(0).isAllIn(),
              "Short raise must put player all-in");
    }

    // Test 5: short all-in bet must be handled
    {
        std::mt19937 deckRng{46};
        std::mt19937 agentRng{103};

        HandEngine engine{
            {1000, 150}, HandConfig{50, 100}, 0, deckRng
        };

        engine.applyAction(0, Action{ActionType::Call, 0});
        engine.applyAction(1, Action{ActionType::Check, 0});

        PlayerObservation obs = engine.observe(1);

        check(obs.legalActions->shortAllInBet.has_value(),
              "Short all-in bet must be available");

        // Force the agent to choose Bet.
        obs.legalActions->canFold = false;
        obs.legalActions->canCheck = false;

        RandomAgent agent{agentRng};
        Action action = agent.chooseAction(obs);

        check(action.type == ActionType::Bet,
              "Agent must select the only available action");

        check(action.amount == 50,
              "Agent must select the correct short all-in bet");

        engine.applyAction(1, action);

        check(engine.state().player(1).isAllIn(),
              "Short bet must put player all-in");
    }

    // Test 6: agent must handle an empty candidate list
    {
        std::mt19937 deckRng{47};
        std::mt19937 agentRng{104};

        HandEngine engine{
            {1000, 1000}, HandConfig{50, 100}, 0, deckRng
        };

        PlayerObservation obs = engine.observe(0);

        obs.legalActions->canFold = false;
        obs.legalActions->canCheck = false;
        obs.legalActions->canCall = false;
        obs.legalActions->canBet = false;
        obs.legalActions->canRaise = false;

        RandomAgent agent{agentRng};

        bool exceptionThrown = false;

        try {
            [[maybe_unused]] Action action = agent.chooseAction(obs);
        }
        catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        check(exceptionThrown,
              "Agent must reject an empty candidate list");
    }

    // Test 7: two random agents must complete a full hand
    {
        std::mt19937 deckRng{48};
        std::mt19937 agentRng0{200};
        std::mt19937 agentRng1{201};

        HandEngine engine{
            {1000, 1000}, HandConfig{50, 100}, 0, deckRng
        };

        RandomAgent agent0{agentRng0};
        RandomAgent agent1{agentRng1};

        std::array<RandomAgent*, 2> agents{
            &agent0, &agent1
        };

        constexpr int maxActions = 100;
        int actionCount = 0;

        while (engine.state().phase() == HandPhase::Betting) {
            check(actionCount < maxActions,
                  "Random agents must not enter an infinite loop");

            const auto actor = engine.state().actingPlayer();
            check(actor.has_value(),
                  "Betting phase must have an acting player");

            const std::uint8_t player = *actor;

            PlayerObservation obs = engine.observe(player);

            check(obs.legalActions.has_value(),
                  "Acting player must receive legal actions");

            Action action = agents[player]->chooseAction(obs);

            // If this throws, RandomAgent generated an illegal action.
            engine.applyAction(player, action);

            ++actionCount;
        }

        check(engine.state().phase() == HandPhase::Finished,
              "Random agents must finish the hand");

        check(actionCount > 0,
              "Random agents must execute at least one action");

        const auto totalChips =
            engine.state().player(0).stack() +
            engine.state().player(1).stack();

        check(totalChips == 2000,
              "Total chips must be conserved after the hand");
    }

    // Test 8: many autonomous hands must terminate legally
    {
        constexpr int numHands = 1000;

        std::mt19937 deckRng{500};
        std::mt19937 agentRng0{501};
        std::mt19937 agentRng1{502};

        RandomAgent agent0{agentRng0};
        RandomAgent agent1{agentRng1};

        std::array<RandomAgent*, 2> agents{
            &agent0, &agent1
        };

        for (int hand = 0; hand < numHands; ++hand) {
            HandEngine engine{
                {1000, 1000},
                HandConfig{50, 100},
                static_cast<std::uint8_t>(hand % 2),
                deckRng
            };

            int actionCount = 0;

            while (engine.state().phase() == HandPhase::Betting) {
                check(actionCount < 100,
                      "Autonomous hand exceeded action limit");

                const auto actor = engine.state().actingPlayer();

                check(actor.has_value(),
                      "Active betting requires an acting player");

                const std::uint8_t player = *actor;

                PlayerObservation obs = engine.observe(player);
                Action action = agents[player]->chooseAction(obs);

                engine.applyAction(player, action);
                ++actionCount;
            }

            check(engine.state().phase() == HandPhase::Finished,
                  "Every autonomous hand must finish");

            const auto totalChips =
                engine.state().player(0).stack() +
                engine.state().player(1).stack();

            check(totalChips == 2000,
                  "Autonomous hands must conserve chips");
        }
    }

    std::cout << "All RandomAgent tests passed!\n";

    return EXIT_SUCCESS;
}
