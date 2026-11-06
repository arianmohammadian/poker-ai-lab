
/**
 * @file legal_actions_test.cc
 * @brief Tests legal action generation for Heads-Up NLHE.
 * @author Arian Mohammadian
 */

#include "poker/hand_engine.h"
#include "poker/legal_actions.h"
#include "poker/action.h"
#include "poker/hand_phase.h"
#include "poker/street.h"

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
using poker::LegalActions;
using poker::Street;

// Test helper
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main() {

    // Test 1: initial preflop actions
    {
        std::mt19937 rng{42};
        HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

        LegalActions legal = engine.legalActions(0);

        check(legal.canFold, "SB must be able to fold");
        check(!legal.canCheck, "SB cannot check facing BB");
        check(legal.canCall, "SB must be able to call");
        check(!legal.canBet, "SB cannot Bet facing BB");
        check(legal.canRaise, "SB must be able to raise");

        check(legal.toCall == 50, "SB must owe 50 chips");
        check(legal.minRaise == 200, "Minimum preflop raise must be 200");
        check(legal.maxRaise == 1000, "Maximum preflop raise must be 1000");

        check(!legal.shortAllInRaise.has_value(),
              "Normal stack must not require short all-in raise");
    }

    // Test 2: big blind after small blind calls
    {
        std::mt19937 rng{43};
        HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

        engine.applyAction(0, Action{ActionType::Call, 0});

        LegalActions legal = engine.legalActions(1);

        check(legal.canFold, "BB can fold");
        check(legal.canCheck, "BB must be able to check");
        check(!legal.canCall, "BB has nothing to call");
        check(!legal.canBet, "BB cannot Bet after blinds");
        check(legal.canRaise, "BB must be able to raise");

        check(legal.toCall == 0, "BB must owe zero");
        check(legal.minRaise == 200, "BB minimum raise must be 200");
        check(legal.maxRaise == 1000, "BB maximum raise must be 1000");

        // The engine must accept a legal minimum raise.
        engine.applyAction(1, Action{ActionType::Raise, legal.minRaise});

        check(engine.state().actingPlayer() == 0,
              "SB must respond to BB raise");
    }

    // Test 3: unopened flop
    {
        std::mt19937 rng{44};
        HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

        engine.applyAction(0, Action{ActionType::Call, 0});
        engine.applyAction(1, Action{ActionType::Check, 0});

        check(engine.state().street() == Street::Flop,
              "Hand must advance to flop");

        LegalActions legal = engine.legalActions(1);

        check(legal.canFold, "Player can fold on flop");
        check(legal.canCheck, "Player can check unopened flop");
        check(!legal.canCall, "Player cannot call unopened flop");
        check(legal.canBet, "Player must be able to bet");
        check(!legal.canRaise, "Player cannot raise unopened flop");

        check(legal.toCall == 0, "Flop toCall must be zero");
        check(legal.minBet == 100, "Minimum flop bet must equal BB");
        check(legal.maxBet == 900, "Maximum flop bet must equal stack");

        check(!legal.shortAllInBet.has_value(),
              "Normal flop bet must not require short all-in");

        // The engine must accept the advertised minimum bet.
        engine.applyAction(1, Action{ActionType::Bet, legal.minBet});

        check(engine.state().actingPlayer() == 0,
              "Other player must respond to flop bet");
    }

    // Test 4: facing a normal flop bet
    {
        std::mt19937 rng{45};
        HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

        engine.applyAction(0, Action{ActionType::Call, 0});
        engine.applyAction(1, Action{ActionType::Check, 0});
        engine.applyAction(1, Action{ActionType::Bet, 100});

        LegalActions legal = engine.legalActions(0);

        check(legal.canFold, "Player can fold facing bet");
        check(!legal.canCheck, "Player cannot check facing bet");
        check(legal.canCall, "Player can call facing bet");
        check(!legal.canBet, "Player cannot Bet facing bet");
        check(legal.canRaise, "Player can raise facing bet");

        check(legal.toCall == 100, "Player must owe 100");
        check(legal.minRaise == 200, "Minimum raise must be 200");
        check(legal.maxRaise == 900, "Maximum raise must be 900");

        // The engine must accept the advertised minimum raise.
        engine.applyAction(0, Action{ActionType::Raise, legal.minRaise});

        check(engine.state().actingPlayer() == 1,
              "Opponent must respond to raise");
    }

    // Test 5: short all-in opening bet
    {
        std::mt19937 rng{46};
        HandEngine engine{{1000, 150}, HandConfig{50, 100}, 0, rng};

        engine.applyAction(0, Action{ActionType::Call, 0});
        engine.applyAction(1, Action{ActionType::Check, 0});

        LegalActions legal = engine.legalActions(1);

        check(legal.canBet, "Short stack must be able to bet all-in");
        check(legal.minBet == 0 && legal.maxBet == 0,
              "No normal bet interval must exist");

        check(legal.shortAllInBet.has_value(),
              "Short all-in bet must be available");

        check(*legal.shortAllInBet == 50,
              "Short all-in opening bet must equal 50");

        // The engine must accept the short all-in bet.
        engine.applyAction(
            1, Action{ActionType::Bet, *legal.shortAllInBet}
        );

        check(engine.state().player(1).isAllIn(),
              "Short opening bet must put player all-in");

        check(engine.state().actingPlayer() == 0,
              "Opponent must respond to short all-in bet");
    }

    // Test 6: incomplete all-in raise
    {
        std::mt19937 rng{47};
        HandEngine engine{{175, 1000}, HandConfig{50, 100}, 0, rng};

        LegalActions legal = engine.legalActions(0);

        check(legal.canRaise, "Incomplete all-in raise must be legal");
        check(legal.minRaise == 0 && legal.maxRaise == 0,
              "No full raise interval must exist");

        check(legal.shortAllInRaise.has_value(),
              "Short all-in raise must be available");

        check(*legal.shortAllInRaise == 175,
              "Short all-in raise must reach 175");

        // The engine must accept the incomplete all-in raise.
        engine.applyAction(
            0, Action{ActionType::Raise, *legal.shortAllInRaise}
        );

        check(engine.state().player(0).isAllIn(),
              "Incomplete raise must put player all-in");

        check(engine.state().lastFullRaise() == 100,
              "Incomplete raise must not change full raise size");
    }

    // Test 7: facing an all-in opponent
    {
        std::mt19937 rng{48};
        HandEngine engine{{1000, 70}, HandConfig{50, 100}, 0, rng};

        LegalActions legal = engine.legalActions(0);

        check(legal.canFold, "Player can fold facing all-in");
        check(!legal.canCheck, "Player cannot check facing all-in");
        check(legal.canCall, "Player can call opponent all-in");

        check(!legal.canBet, "Cannot bet against all-in opponent");
        check(!legal.canRaise, "Cannot raise against all-in opponent");

        check(legal.toCall == 20, "Player must owe 20 chips");

        check(!legal.shortAllInRaise.has_value(),
              "No short all-in raise against all-in opponent");

        engine.applyAction(0, Action{ActionType::Call, 0});

        check(engine.state().phase() == HandPhase::Finished,
              "Calling final all-in must finish hand");
    }

    // Test 8: partial all-in call
    {
        std::mt19937 rng{49};
        HandEngine engine{{1000, 200}, HandConfig{50, 100}, 0, rng};

        engine.applyAction(0, Action{ActionType::Raise, 500});

        LegalActions legal = engine.legalActions(1);

        check(legal.canFold, "Short stack can fold");
        check(!legal.canCheck, "Short stack cannot check");
        check(legal.canCall, "Partial all-in call must be legal");
        check(!legal.canBet, "Cannot bet facing raise");
        check(!legal.canRaise, "Cannot raise without enough chips");

        check(legal.toCall == 400,
              "toCall must represent full amount owed");

        check(engine.state().player(1).stack() == 100,
              "Short player must have 100 chips remaining");

        engine.applyAction(1, Action{ActionType::Call, 0});

        check(engine.state().phase() == HandPhase::Finished,
              "Partial all-in call must resolve hand");
    }

    // Test 9: invalid player and invalid turn
    {
        std::mt19937 rng{50};
        HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

        bool invalidPlayerThrown = false;

        try {
            [[maybe_unused]] auto legal = engine.legalActions(2);
        }
        catch (const std::invalid_argument&) {
            invalidPlayerThrown = true;
        }

        check(invalidPlayerThrown,
              "Invalid player index must throw");

        bool wrongTurnThrown = false;

        try {
            [[maybe_unused]] auto legal = engine.legalActions(1);
        }
        catch (const std::invalid_argument&) {
            wrongTurnThrown = true;
        }

        check(wrongTurnThrown,
              "Observing legal actions on wrong turn must throw");

        engine.applyAction(0, Action{ActionType::Fold, 0});

        bool finishedHandThrown = false;

        try {
            [[maybe_unused]] auto legal = engine.legalActions(0);
        }
        catch (const std::invalid_argument&) {
            finishedHandThrown = true;
        }

        check(finishedHandThrown,
              "Legal actions after hand finishes must throw");
    }

    // Test 10: legal actions must not mutate game state
    {
        std::mt19937 rng{51};
        HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

        const auto pot = engine.state().pot();
        const auto stack0 = engine.state().player(0).stack();
        const auto stack1 = engine.state().player(1).stack();
        const auto actingPlayer = engine.state().actingPlayer();

        [[maybe_unused]] auto legal = engine.legalActions(0);

        check(engine.state().pot() == pot,
              "Computing legal actions must not modify pot");

        check(engine.state().player(0).stack() == stack0,
              "Computing legal actions must not modify player 0 stack");

        check(engine.state().player(1).stack() == stack1,
              "Computing legal actions must not modify player 1 stack");

        check(engine.state().actingPlayer() == actingPlayer,
              "Computing legal actions must not change acting player");
    }

    // Test 11: raise boundaries must match engine validation
      {
      std::mt19937 rng{53};
      HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

      LegalActions legal = engine.legalActions(0);

      check(legal.minRaise == 200,
            "Minimum raise must be 200");

      check(legal.maxRaise == 1000,
            "Maximum raise must be 1000");

      bool belowMinimumRejected = false;

      try {
            engine.applyAction(0, Action{ActionType::Raise, 199});
      }
      catch (const std::invalid_argument&) {
            belowMinimumRejected = true;
      }

      check(belowMinimumRejected,
            "Raise below minimum must be rejected");

      bool aboveMaximumRejected = false;

      try {
            engine.applyAction(0, Action{ActionType::Raise, 1001});
      }
      catch (const std::invalid_argument&) {
            aboveMaximumRejected = true;
      }

      check(aboveMaximumRejected,
            "Raise above maximum must be rejected");

      check(engine.state().actingPlayer() == 0,
            "Rejected raises must not change acting player");

      engine.applyAction(
            0, Action{ActionType::Raise, legal.maxRaise}
      );

      check(engine.state().player(0).isAllIn(),
            "Maximum raise must be accepted as all-in");
      }

      // Test 12: bet boundaries must match engine validation
      {
      std::mt19937 rng{54};
      HandEngine engine{{1000, 1000}, HandConfig{50, 100}, 0, rng};

      engine.applyAction(0, Action{ActionType::Call, 0});
      engine.applyAction(1, Action{ActionType::Check, 0});

      LegalActions legal = engine.legalActions(1);

      check(legal.minBet == 100,
            "Minimum flop bet must equal big blind");

      check(legal.maxBet == 900,
            "Maximum flop bet must equal remaining stack");

      bool belowMinimumRejected = false;

      try {
            engine.applyAction(1, Action{ActionType::Bet, 99});
      }
      catch (const std::invalid_argument&) {
            belowMinimumRejected = true;
      }

      check(belowMinimumRejected,
            "Non-all-in bet below minimum must be rejected");

      bool aboveMaximumRejected = false;

      try {
            engine.applyAction(1, Action{ActionType::Bet, 901});
      }
      catch (const std::invalid_argument&) {
            aboveMaximumRejected = true;
      }

      check(aboveMaximumRejected,
            "Bet above stack must be rejected");

      engine.applyAction(
            1, Action{ActionType::Bet, legal.minBet}
      );

      check(engine.state().actingPlayer() == 0,
            "Minimum legal bet must be accepted");
      }

    std::cout << "All LegalActions tests passed!\n";

    return EXIT_SUCCESS;
}
