
/**
 * @file game_engine_test.cc
 * @brief Tests Heads-Up NLHE game session management.
 * @author Arian Mohammadian
 */

#include "poker/game_engine.h"
#include "poker/random_agent.h"
#include "poker/action.h"
#include "poker/hand_phase.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using poker::Action;
using poker::ActionType;
using poker::GameEngine;
using poker::HandConfig;
using poker::HandPhase;
using poker::RandomAgent;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

// Total chips, including chips currently committed to the pot.
std::int64_t totalChips(const GameEngine& game) {
    const auto& state = game.currentHand().state();

    return state.player(0).stack() +
           state.player(1).stack() +
           state.pot();
}

// Run a reproducible autonomous session and record its actions.
std::vector<std::string> runSession() {
    std::mt19937 deckRng{500};
    std::mt19937 agentRng0{501};
    std::mt19937 agentRng1{502};

    GameEngine game{
        {1000, 1000}, HandConfig{50, 100}, 0, deckRng
    };

    RandomAgent agent0{agentRng0};
    RandomAgent agent1{agentRng1};

    std::array<RandomAgent*, 2> agents{&agent0, &agent1};

    std::vector<std::string> history;

    constexpr std::uint64_t maxHands = 100;
    constexpr int maxActionsPerHand = 100;

    while (!game.isFinished() && game.handNumber() <= maxHands) {
        int actionCount = 0;

        while (game.currentHand().state().phase() == HandPhase::Betting) {
            check(actionCount < maxActionsPerHand,
                  "Autonomous hand exceeded action limit");

            const auto actor =
                game.currentHand().state().actingPlayer();

            check(actor.has_value(),
                  "Betting phase requires an acting player");

            const std::uint8_t player = *actor;

            const auto obs = game.currentHand().observe(player);
            Action action = agents[player]->chooseAction(obs);

            history.push_back(
                std::to_string(game.handNumber()) + ":" +
                std::to_string(player) + ":" +
                std::to_string(static_cast<int>(action.type)) + ":" +
                std::to_string(action.amount)
            );

            game.applyAction(player, action);

            check(totalChips(game) == 2000,
                  "Autonomous session must conserve chips");

            ++actionCount;
        }

        const auto& state = game.currentHand().state();

        check(state.phase() == HandPhase::Finished,
              "Every autonomous hand must finish");

        check(state.pot() == 0,
              "Finished hand must have no outstanding pot");

        history.push_back(
            "END:" + std::to_string(game.handNumber()) + ":" +
            std::to_string(state.player(0).stack()) + ":" +
            std::to_string(state.player(1).stack())
        );

        if (!game.isFinished() && game.handNumber() < maxHands) {
            game.nextHand();
            check(totalChips(game) == 2000,
                  "Starting next hand must conserve chips");
        } else {
            break;
        }
    }

    check(!history.empty(),
          "Autonomous session must produce history");

    return history;
}

int main() {

    // Test 1: initial game state
    {
        std::mt19937 rng{42};

        GameEngine game{
            {1000, 1000}, HandConfig{50, 100}, 0, rng
        };

        const auto& state = game.currentHand().state();

        check(game.handNumber() == 1,
              "First hand number must be 1");

        check(!game.isFinished(),
              "New game must not be finished");

        check(state.button() == 0,
              "Initial button must be player 0");

        check(state.phase() == HandPhase::Betting,
              "Initial hand must accept actions");

        check(state.player(0).stack() == 950,
              "Player 0 must post the small blind");

        check(state.player(1).stack() == 900,
              "Player 1 must post the big blind");

        check(totalChips(game) == 2000,
              "Initial chips must be conserved");
    }

    // Test 2: cannot advance an unfinished hand
    {
        std::mt19937 rng{43};
        GameEngine game{
            {1000, 1000}, HandConfig{50, 100}, 0, rng
        };

        bool exceptionThrown = false;

        try {
            game.nextHand();
        }
        catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        check(exceptionThrown,
              "Cannot start next hand before current hand finishes");

        check(game.handNumber() == 1,
              "Rejected transition must not change hand number");

        check(game.currentHand().state().button() == 0,
              "Rejected transition must not change button");
    }

    // Test 3: stacks persist between hands
    {
        std::mt19937 rng{44};
        GameEngine game{
            {1000, 1000}, HandConfig{50, 100}, 0, rng
        };

        game.applyAction(0, Action{ActionType::Fold, 0});

        const auto& finished = game.currentHand().state();

        check(finished.phase() == HandPhase::Finished,
              "Fold must finish current hand");

        check(finished.player(0).stack() == 950,
              "Folding small blind must lose 50 chips");

        check(finished.player(1).stack() == 1050,
              "Big blind must win the small blind");

        check(!game.isFinished(),
              "Game must continue while both players have chips");

        game.nextHand();

        const auto& next = game.currentHand().state();

        check(next.button() == 1,
              "Button must alternate to player 1");

        check(next.player(0).stack() == 850,
              "Player 0 must post big blind from 950");

        check(next.player(1).stack() == 1000,
              "Player 1 must post small blind from 1050");

        check(next.pot() == 150,
              "Next hand must start with both blinds");

        check(totalChips(game) == 2000,
              "Stacks must persist without losing chips");
    }

    // Test 4: button alternates over multiple hands
    {
        std::mt19937 rng{45};
        GameEngine game{
            {1000, 1000}, HandConfig{50, 100}, 0, rng
        };

        for (int hand = 1; hand <= 10; ++hand) {
            const auto& state = game.currentHand().state();

            check(state.button() == static_cast<std::uint8_t>(
                      (hand - 1) % 2),
                  "Button must alternate every hand");

            check(game.handNumber() ==
                      static_cast<std::uint64_t>(hand),
                  "Hand counter must match current hand");

            const auto actor = state.actingPlayer();

            check(actor.has_value(),
                  "Small blind must act first preflop");

            check(*actor == state.button(),
                  "Button must act first preflop");

            game.applyAction(*actor, Action{ActionType::Fold, 0});

            check(totalChips(game) == 2000,
                  "Repeated folds must conserve chips");

            check(!game.isFinished(),
                  "Ten alternating folds must not end the game");

            if (hand < 10) {
                game.nextHand();
            }
        }
    }

    // Test 5: actions are delegated to HandEngine
    {
        std::mt19937 rng{46};
        GameEngine game{
            {1000, 1000}, HandConfig{50, 100}, 0, rng
        };

        game.applyAction(0, Action{ActionType::Call, 0});

        check(game.currentHand().state().actingPlayer() == 1,
              "GameEngine must delegate Call correctly");

        check(game.currentHand().state().pot() == 200,
              "Call must update the pot");

        game.applyAction(1, Action{ActionType::Check, 0});

        check(game.currentHand().state().street() ==
                  poker::Street::Flop,
              "GameEngine must delegate Check and advance street");

        check(totalChips(game) == 2000,
              "Delegated actions must conserve chips");
    }

    // Test 6: actions must be rejected after a finished hand
    {
        std::mt19937 rng{47};
        GameEngine game{
            {1000, 1000}, HandConfig{50, 100}, 0, rng
        };

        game.applyAction(0, Action{ActionType::Fold, 0});

        bool exceptionThrown = false;

        try {
            game.applyAction(1, Action{ActionType::Check, 0});
        }
        catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        check(exceptionThrown,
              "Finished hand must reject additional actions");

        check(game.handNumber() == 1,
              "Rejected action must not advance game");
    }

    // Test 7: finished game after a player loses all chips
    {
        // A player starting with exactly the SB is all-in
        // when blinds are posted. Find a deterministic seed
        // where the short stack loses the showdown.
        bool foundFinishedGame = false;

        for (std::uint32_t seed = 0; seed < 256; ++seed) {
            std::mt19937 rng{seed};

            GameEngine game{
                {50, 1000}, HandConfig{50, 100}, 0, rng
            };

            if (!game.isFinished()) {
                continue;
            }

            foundFinishedGame = true;

            check(game.handNumber() == 1,
                  "Game must finish during first hand");

            check(game.currentHand().state().phase() ==
                      HandPhase::Finished,
                  "Final hand must be settled");

            check(game.currentHand().state().player(0).stack() == 0,
                  "Eliminated player must have zero chips");

            check(totalChips(game) == 1050,
                  "Elimination must conserve all chips");

            bool exceptionThrown = false;

            try {
                game.nextHand();
            }
            catch (const std::logic_error&) {
                exceptionThrown = true;
            }

            check(exceptionThrown,
                  "Cannot start another hand after game ends");

            check(game.handNumber() == 1,
                  "Finished game must not increment hand counter");

            break;
        }

        check(foundFinishedGame,
              "At least one seed must eliminate the short stack");
    }

    // Test 8: identical seeds produce identical game histories
    {
        const auto history1 = runSession();
        const auto history2 = runSession();

        check(history1 == history2,
              "Identical seeds must reproduce complete game history");
    }

    // Test 9: invalid constructor arguments are rejected
    {
        std::mt19937 rng{48};

        bool invalidButtonThrown = false;

        try {
            [[maybe_unused]] GameEngine game{
                {1000, 1000}, HandConfig{50, 100}, 2, rng
            };
        }
        catch (const std::invalid_argument&) {
            invalidButtonThrown = true;
        }

        check(invalidButtonThrown,
              "Invalid initial button must be rejected");

        bool invalidStackThrown = false;

        try {
            [[maybe_unused]] GameEngine game{
                {0, 1000}, HandConfig{50, 100}, 0, rng
            };
        }
        catch (const std::invalid_argument&) {
            invalidStackThrown = true;
        }

        check(invalidStackThrown,
              "Zero initial stack must be rejected");
    }

    // Test 10: repeated session transitions preserve all chips
    {
        std::mt19937 rng{49};

        GameEngine game{
            {10000, 10000}, HandConfig{50, 100}, 0, rng
        };

        constexpr int numHands = 100;

        for (int i = 0; i < numHands; ++i) {
            check(game.handNumber() ==
                      static_cast<std::uint64_t>(i + 1),
                  "Sequential hand counter must be correct");

            check(totalChips(game) == 20000,
                  "Chip total must remain constant");

            const auto actor =
                game.currentHand().state().actingPlayer();

            check(actor.has_value(),
                  "Current hand must have an acting player");

            // Deliberately fold the small blind to keep
            // this stress test independent from agent RNG.
            game.applyAction(*actor, Action{ActionType::Fold, 0});

            check(game.currentHand().state().phase() ==
                      HandPhase::Finished,
                  "Each hand must finish after fold");

            check(totalChips(game) == 20000,
                  "Finished hands must conserve chips");

            if (i + 1 < numHands) {
                check(!game.isFinished(),
                      "Game must remain active");

                game.nextHand();
            }
        }

        check(game.handNumber() == numHands,
              "Game must complete 100 consecutive hands");
    }

    std::cout << "All GameEngine tests passed!\n";

    return EXIT_SUCCESS;
}
