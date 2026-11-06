
/**
 * @file player_observation_test.cc
 * @brief Tests player observations and private information isolation.
 * @author Arian Mohammadian
 */

#include "poker/hand_engine.h"
#include "poker/player_observation.h"
#include "poker/action.h"

#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>

using poker::Action;
using poker::ActionType;
using poker::HandConfig;
using poker::HandEngine;
using poker::PlayerObservation;
using poker::Street;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    std::mt19937 rng{42};

    HandEngine engine{
        {1000, 1000},
        HandConfig{50, 100},
        0,
        rng
    };

    const auto& state = engine.state();

    PlayerObservation obs0 = engine.observe(0);
    PlayerObservation obs1 = engine.observe(1);

    // Test 1: player identity
    check(obs0.player == 0,
          "Observation 0 must belong to player 0");

    check(obs1.player == 1,
          "Observation 1 must belong to player 1");

    // Test 2: private hole cards
    check(obs0.holeCards == state.player(0).holeCards(),
          "Player 0 must observe their own hole cards");

    check(obs1.holeCards == state.player(1).holeCards(),
          "Player 1 must observe their own hole cards");

    check(obs0.holeCards != state.player(1).holeCards(),
          "Player 0 must not receive player 1 hole cards");

    check(obs1.holeCards != state.player(0).holeCards(),
          "Player 1 must not receive player 0 hole cards");

    // Test 3: public information
    check(obs0.board.empty() && obs1.board.empty(),
          "Preflop board must be empty");

    check(obs0.street == Street::Preflop,
          "Initial street must be preflop");

    check(obs0.button == 0 && obs1.button == 0,
          "Both observations must have the same button");

    check(obs0.pot == 150 && obs1.pot == 150,
          "Initial pot must contain both blinds");

    check(obs0.stacks[0] == 950 && obs0.stacks[1] == 900,
          "Observation must contain correct player stacks");

    check(obs1.stacks == obs0.stacks,
          "Both observations must contain the same stacks");

    check(obs0.committedStreet[0] == 50,
          "Player 0 must have committed the small blind");

    check(obs0.committedStreet[1] == 100,
          "Player 1 must have committed the big blind");

    check(obs1.committedStreet == obs0.committedStreet,
          "Both observations must contain public commitments");

    // Test 4: invalid player index
    bool exceptionThrown = false;

    try {
        [[maybe_unused]] auto invalid = engine.observe(2);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    check(exceptionThrown,
          "Observing player 2 must throw invalid_argument");

    // Test 5: observation must be an independent copy
    const auto originalPot = engine.state().pot();
    const auto originalStack = engine.state().player(0).stack();
    const auto originalCard = engine.state().player(0).holeCards()[0];

    obs0.pot = 999999;
    obs0.stacks[0] = 0;
    obs0.holeCards[0] = poker::Card{
        poker::Rank::Two,
        poker::Suit::Clubs
    };

    check(engine.state().pot() == originalPot,
          "Modifying observation must not change engine pot");

    check(engine.state().player(0).stack() == originalStack,
          "Modifying observation must not change engine stack");

    check(engine.state().player(0).holeCards()[0] == originalCard,
          "Modifying observation must not change engine cards");

    // Test 6: observations after advancing to flop
    engine.applyAction(0, Action{ActionType::Call, 0});
    engine.applyAction(1, Action{ActionType::Check, 0});

    PlayerObservation flop0 = engine.observe(0);
    PlayerObservation flop1 = engine.observe(1);

    check(flop0.street == Street::Flop,
          "Observation must reflect current street");

    check(flop0.board.size() == 3,
          "Flop observation must contain three board cards");

    check(flop0.board == flop1.board,
          "Both players must observe the same board");

    check(flop0.board == engine.state().board(),
          "Observed board must match engine board");

    check(flop0.pot == 200 && flop1.pot == 200,
          "Flop pot must contain both completed preflop bets");

    check(flop0.committedStreet[0] == 0 &&
          flop0.committedStreet[1] == 0,
          "Street commitments must reset on flop");

    check(obs1.board.empty(),
          "Previous observation must not change after street transition");

    // Test 7: deterministic observations
    std::mt19937 rng2{42};

    HandEngine engine2{
        {1000, 1000},
        HandConfig{50, 100},
        0,
        rng2
    };

    PlayerObservation deterministic = engine2.observe(0);

    check(deterministic.holeCards ==
          engine.state().player(0).holeCards(),
          "Same seed must produce the same hole cards");

    check(deterministic.pot == 150,
          "Recreated hand must have the same initial pot");

      // Test 8: legal actions in player observations
      {
      std::mt19937 testRng{52};
      HandEngine testEngine{
            {1000, 1000}, HandConfig{50, 100}, 0, testRng
      };

      auto first = testEngine.observe(0);
      auto waiting = testEngine.observe(1);

      check(first.legalActions.has_value(),
            "Acting player must receive legal actions");

      check(!waiting.legalActions.has_value(),
            "Waiting player must not receive legal actions");

      check(first.legalActions->canCall,
            "Small blind must be able to call");

      check(first.legalActions->minRaise == 200,
            "Observed minimum raise must be correct");

      testEngine.applyAction(0, Action{ActionType::Call, 0});

      auto afterCall0 = testEngine.observe(0);
      auto afterCall1 = testEngine.observe(1);

      check(!afterCall0.legalActions.has_value(),
            "Player 0 must have no actions when waiting");

      check(afterCall1.legalActions.has_value(),
            "Player 1 must receive legal actions on their turn");

      check(afterCall1.legalActions->canCheck,
            "Big blind must be able to check");

      testEngine.applyAction(1, Action{ActionType::Fold, 0});

      check(!testEngine.observe(0).legalActions.has_value(),
            "Finished hand must have no legal actions for player 0");

      check(!testEngine.observe(1).legalActions.has_value(),
            "Finished hand must have no legal actions for player 1");
      }

    std::cout << "All PlayerObservation tests passed!\n";

    return EXIT_SUCCESS;
}
