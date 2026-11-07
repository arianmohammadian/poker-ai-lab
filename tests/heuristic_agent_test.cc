
/**
 * @file heuristic_agent_test.cc
 * @brief Tests for passive and aggressive equity-based agents.
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
#include <limits>
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
using poker::HeuristicConfig;
using poker::HeuristicMode;
using poker::LegalActions;
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
    return Card(
        static_cast<Rank>(rank),
        static_cast<Suit>(suit)
    );
}

void checkAction(
    Action action,
    ActionType expected,
    const char* message
) {
    check(action.type == expected, message);
    check(action.amount == 0,
          "Passive action amount must be zero");
}

void checkAggressiveAction(
    Action action,
    ActionType expected,
    std::int64_t expectedAmount,
    const char* message
) {
    check(action.type == expected, message);
    check(action.amount == expectedAmount,
          "Incorrect aggressive action amount");
}

void checkLegalAction(
    const Action& action,
    const LegalActions& legal
) {
    switch (action.type) {
        case ActionType::Fold:
            check(legal.canFold, "Illegal Fold");
            check(action.amount == 0,
                  "Fold amount must be zero");
            break;

        case ActionType::Check:
            check(legal.canCheck, "Illegal Check");
            check(action.amount == 0,
                  "Check amount must be zero");
            break;

        case ActionType::Call:
            check(legal.canCall, "Illegal Call");
            check(action.amount == 0,
                  "Call amount must be zero");
            break;

        case ActionType::Bet:
            check(legal.canBet, "Illegal Bet");

            check(
                (legal.shortAllInBet.has_value() &&
                action.amount == *legal.shortAllInBet) ||
                (legal.minBet > 0 &&
                action.amount >= legal.minBet &&
                action.amount <= legal.maxBet),
                "Bet outside legal bounds"
            );
            break;

        case ActionType::Raise:
            check(legal.canRaise, "Illegal Raise");

            check(
                (legal.shortAllInRaise.has_value() &&
                action.amount == *legal.shortAllInRaise) ||
                (legal.minRaise > 0 &&
                action.amount >= legal.minRaise &&
                action.amount <= legal.maxRaise),
                "Raise outside legal bounds"
            );
            break;

        default:
            check(false, "Unknown action type");
    }
}

HeuristicConfig aggressiveConfig() {
    HeuristicConfig config;
    config.mode = HeuristicMode::Aggressive;
    config.equitySamples = 100;
    config.bluffFrequency = 0.0;
    return config;
}

// Real preflop observation: small blind facing big blind.
PlayerObservation initialObservation(
    std::uint32_t seed = 42
) {
    std::mt19937 deckRng(seed);

    HandEngine engine{
        {1000, 1000},
        HandConfig{50, 100},
        0,
        deckRng
    };

    return engine.observe(0);
}

// Hero has an unbeatable royal flush.
// The two remaining board cards do not create a tie.
void setGuaranteedWin(PlayerObservation& observation) {
    observation.holeCards = {
        card(14, 3), card(13, 3)
    };

    observation.board = {
        card(12, 3), card(11, 3), card(10, 3),
        card(2, 0), card(3, 1)
    };
}

// Royal flush entirely on the board.
// Every possible opponent hand ties with the hero.
void setGuaranteedTie(PlayerObservation& observation) {
    observation.holeCards = {
        card(2, 0), card(3, 1)
    };

    observation.board = {
        card(10, 3), card(11, 3), card(12, 3),
        card(13, 3), card(14, 3)
    };
}

// Controlled state with Check and Bet available.
PlayerObservation bettingObservation() {
    auto observation = initialObservation(200);

    setGuaranteedWin(observation);

    observation.pot = 400;

    auto& legal = *observation.legalActions;
    legal.canCheck = true;
    legal.canCall = false;
    legal.canBet = true;
    legal.canRaise = false;
    legal.minBet = 100;
    legal.maxBet = 500;

    return observation;
}

// Controlled state with Call and Raise available.
PlayerObservation raisingObservation() {
    auto observation = initialObservation(201);

    setGuaranteedWin(observation);

    observation.player = 0;
    observation.committedStreet = {50, 100};
    observation.pot = 600;

    auto& legal = *observation.legalActions;
    legal.canCheck = false;
    legal.canCall = true;
    legal.canBet = false;
    legal.canRaise = true;
    legal.toCall = 50;
    legal.minRaise = 200;
    legal.maxRaise = 700;

    return observation;
}

// ============================================================
// H3.1 — Configuration and passive policy
// ============================================================

void testNoLegalActions() {
    std::mt19937 deckRng{42};
    std::mt19937 agentRng{100};

    HandEngine engine{
        {1000, 1000},
        HandConfig{50, 100},
        0,
        deckRng
    };

    HeuristicAgent agent(agentRng);

    const auto observation = engine.observe(1);

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

void testInvalidConfig() {
    std::mt19937 rng{101};
    const auto base = aggressiveConfig();

    auto expectInvalid = [&](HeuristicConfig config) {
        bool threw = false;

        try {
            HeuristicAgent agent(rng, config);
        } catch (const std::invalid_argument&) {
            threw = true;
        }

        check(threw, "Invalid configuration must be rejected");
    };

    auto config = base;
    config.equitySamples = 0;
    expectInvalid(config);

    config = base;
    config.valueBetThreshold = -0.1;
    expectInvalid(config);

    config = base;
    config.valueRaiseThreshold = 1.1;
    expectInvalid(config);

    config = base;
    config.bluffFrequency = -0.1;
    expectInvalid(config);

    config = base;
    config.bluffFrequency = 1.1;
    expectInvalid(config);

    config = base;
    config.betPotFraction = 0.0;
    expectInvalid(config);

    config = base;
    config.raisePotFraction = -1.0;
    expectInvalid(config);

    config = base;
    config.valueBetThreshold =
        std::numeric_limits<double>::quiet_NaN();
    expectInvalid(config);

    config = base;
    config.betPotFraction =
        std::numeric_limits<double>::infinity();
    expectInvalid(config);

    // Boundary probabilities are valid.
    config = base;
    config.bluffFrequency = 0.0;
    HeuristicAgent zeroBluff(rng, config);

    config.bluffFrequency = 1.0;
    HeuristicAgent alwaysBluff(rng, config);

    (void)zeroBluff;
    (void)alwaysBluff;
}

void testPassiveCheck() {
    std::mt19937 deckRng{43};
    std::mt19937 agentRng{102};

    HandEngine engine{
        {1000, 1000},
        HandConfig{50, 100},
        0,
        deckRng
    };

    HeuristicAgent agent(agentRng);

    engine.applyAction(0, Action{ActionType::Call, 0});

    const auto observation = engine.observe(1);

    check(observation.legalActions.has_value() &&
          observation.legalActions->canCheck,
          "Big blind must be allowed to check after a limp");

    const auto before = agentRng;

    checkAction(
        agent.chooseAction(observation),
        ActionType::Check,
        "Passive agent must check when checking is legal"
    );

    check(agentRng == before,
          "Passive Check must not consume randomness");
}

void testGuaranteedCall() {
    std::mt19937 rng{103};
    HeuristicAgent agent(rng);

    auto observation = initialObservation(44);
    setGuaranteedWin(observation);

    checkAction(
        agent.chooseAction(observation),
        ActionType::Call,
        "Unbeatable hero must Call"
    );
}

void testFoldWhenPotOddsAreBad() {
    std::mt19937 rng{104};
    HeuristicAgent agent(rng);

    auto observation = initialObservation(45);
    setGuaranteedTie(observation);

    observation.pot = 100;

    auto& legal = *observation.legalActions;
    legal.canCheck = false;
    legal.canCall = true;
    legal.toCall = 300;

    // Equity = 50%, required equity = 75%.
    checkAction(
        agent.chooseAction(observation),
        ActionType::Fold,
        "Agent must Fold when pot odds are unfavorable"
    );
}

void testCallWhenPotOddsAreGood() {
    std::mt19937 rng{105};
    HeuristicAgent agent(rng);

    auto observation = initialObservation(46);
    setGuaranteedTie(observation);

    observation.pot = 300;

    auto& legal = *observation.legalActions;
    legal.canCheck = false;
    legal.canCall = true;
    legal.toCall = 100;

    // Equity = 50%, required equity = 25%.
    checkAction(
        agent.chooseAction(observation),
        ActionType::Call,
        "Agent must Call when pot odds are favorable"
    );
}

void testPassiveNoCallAvailable() {
    std::mt19937 rng{106};
    HeuristicAgent agent(rng);

    auto observation = initialObservation(49);

    auto& legal = *observation.legalActions;
    legal.canCheck = false;
    legal.canCall = false;
    legal.canFold = true;

    const auto before = rng;

    checkAction(
        agent.chooseAction(observation),
        ActionType::Fold,
        "Passive agent must Fold without Check or Call"
    );

    check(rng == before,
          "Immediate Fold must not consume randomness");
}

void testPassiveNeverBetsOrRaises() {
    std::mt19937 rng{107};
    HeuristicAgent agent(rng);

    auto betObservation = bettingObservation();

    checkAction(
        agent.chooseAction(betObservation),
        ActionType::Check,
        "Passive agent must not Bet"
    );

    auto raiseObservation = raisingObservation();

    checkAction(
        agent.chooseAction(raiseObservation),
        ActionType::Call,
        "Passive agent must not Raise"
    );
}

// ============================================================
// H3.2 — Value betting and bet sizing
// ============================================================

void testValueBet() {
    std::mt19937 rng{200};
    HeuristicAgent agent(rng, aggressiveConfig());

    const auto observation = bettingObservation();

    const Action action = agent.chooseAction(observation);

    // 50% of pot 400 = 200.
    checkAggressiveAction(
        action,
        ActionType::Bet,
        200,
        "Strong hero must make a value Bet"
    );

    checkLegalAction(action, *observation.legalActions);
}

void testBetSizingBounds() {
    std::mt19937 rng{201};
    auto config = aggressiveConfig();

    // Target: 20. Minimum legal Bet: 100.
    config.betPotFraction = 0.05;

    {
        HeuristicAgent agent(rng, config);
        auto observation = bettingObservation();

        checkAggressiveAction(
            agent.chooseAction(observation),
            ActionType::Bet,
            100,
            "Bet must respect minimum"
        );
    }

    // Target: 800. Maximum legal Bet: 500.
    config.betPotFraction = 2.0;

    {
        HeuristicAgent agent(rng, config);
        auto observation = bettingObservation();

        checkAggressiveAction(
            agent.chooseAction(observation),
            ActionType::Bet,
            500,
            "Bet must respect maximum"
        );
    }
}

// ============================================================
// H3.3 — Value raising and raise sizing
// ============================================================

void testValueRaise() {
    std::mt19937 rng{300};
    HeuristicAgent agent(rng, aggressiveConfig());

    const auto observation = raisingObservation();

    const Action action = agent.chooseAction(observation);

    // Opponent commitment: 100.
    // Raise increment: 0.75 * 600 = 450.
    // Total target: 550.
    checkAggressiveAction(
        action,
        ActionType::Raise,
        550,
        "Strong hero must make a value Raise"
    );

    checkLegalAction(action, *observation.legalActions);
}

void testRaiseFromPlayerOne() {
    std::mt19937 rng{301};
    HeuristicAgent agent(rng, aggressiveConfig());

    auto observation = raisingObservation();

    observation.player = 1;
    observation.committedStreet = {175, 50};
    observation.legalActions->toCall = 125;
    observation.legalActions->minRaise = 250;
    observation.legalActions->maxRaise = 750;

    // Opponent is player 0, committed 175.
    // Target: 175 + 450 = 625.
    checkAggressiveAction(
        agent.chooseAction(observation),
        ActionType::Raise,
        625,
        "Raise must use the correct opponent commitment"
    );
}

void testRaiseSizingBounds() {
    std::mt19937 rng{302};
    auto config = aggressiveConfig();

    // Target: 100 + 0.01 * 600 = 106.
    config.raisePotFraction = 0.01;

    {
        HeuristicAgent agent(rng, config);
        auto observation = raisingObservation();

        checkAggressiveAction(
            agent.chooseAction(observation),
            ActionType::Raise,
            200,
            "Raise must respect minimum"
        );
    }

    // Target: 100 + 2 * 600 = 1300.
    config.raisePotFraction = 2.0;

    {
        HeuristicAgent agent(rng, config);
        auto observation = raisingObservation();

        checkAggressiveAction(
            agent.chooseAction(observation),
            ActionType::Raise,
            700,
            "Raise must respect maximum"
        );
    }
}

// ============================================================
// H3.4 — Bluffing and action legality
// ============================================================

void testBluffBetFrequencyZero() {
    std::mt19937 rng{400};
    auto config = aggressiveConfig();
    config.bluffFrequency = 0.0;

    HeuristicAgent agent(rng, config);

    auto observation = bettingObservation();
    setGuaranteedTie(observation);

    // Equity 50% < valueBetThreshold 70%.
    for (int i = 0; i < 20; ++i) {
        checkAction(
            agent.chooseAction(observation),
            ActionType::Check,
            "Zero bluff frequency must never bluff Bet"
        );
    }
}

void testBluffBetFrequencyOne() {
    std::mt19937 rng{401};
    auto config = aggressiveConfig();
    config.bluffFrequency = 1.0;

    HeuristicAgent agent(rng, config);

    auto observation = bettingObservation();
    setGuaranteedTie(observation);

    for (int i = 0; i < 20; ++i) {
        const Action action = agent.chooseAction(observation);

        checkAggressiveAction(
            action,
            ActionType::Bet,
            200,
            "Bluff frequency one must always Bet"
        );

        checkLegalAction(action, *observation.legalActions);
    }
}

void testBluffRaiseFrequencyZero() {
    std::mt19937 rng{402};
    auto config = aggressiveConfig();
    config.bluffFrequency = 0.0;

    HeuristicAgent agent(rng, config);

    auto observation = raisingObservation();
    setGuaranteedTie(observation);

    // Required equity: 50 / (600 + 50) < 50%.
    for (int i = 0; i < 20; ++i) {
        checkAction(
            agent.chooseAction(observation),
            ActionType::Call,
            "Zero bluff frequency must not bluff Raise"
        );
    }
}

void testBluffRaiseFrequencyOne() {
    std::mt19937 rng{403};
    auto config = aggressiveConfig();
    config.bluffFrequency = 1.0;

    HeuristicAgent agent(rng, config);

    auto observation = raisingObservation();
    setGuaranteedTie(observation);

    for (int i = 0; i < 20; ++i) {
        const Action action = agent.chooseAction(observation);

        checkAggressiveAction(
            action,
            ActionType::Raise,
            550,
            "Bluff frequency one must always Raise"
        );

        checkLegalAction(action, *observation.legalActions);
    }
}

void testNoIllegalBluff() {
    std::mt19937 rng{404};
    auto config = aggressiveConfig();
    config.bluffFrequency = 1.0;

    HeuristicAgent agent(rng, config);

    auto observation = bettingObservation();
    setGuaranteedTie(observation);

    auto& legal = *observation.legalActions;
    legal.canBet = false;
    legal.canRaise = false;
    legal.canCheck = true;
    legal.canCall = false;

    for (int i = 0; i < 20; ++i) {
        const Action action = agent.chooseAction(observation);

        checkAction(
            action,
            ActionType::Check,
            "Bluff must not bypass action legality"
        );

        checkLegalAction(action, legal);
    }
}

void testCheckWithRaiseAvailable() {
    std::mt19937 rng{405};
    auto config = aggressiveConfig();
    config.bluffFrequency = 0.0;

    HeuristicAgent agent(rng, config);

    auto observation = raisingObservation();
    setGuaranteedWin(observation);

    // Big blind can check after a limp, but can also raise.
    auto& legal = *observation.legalActions;
    legal.canCheck = true;
    legal.canCall = false;
    legal.canRaise = true;

    const Action action = agent.chooseAction(observation);

    check(action.type == ActionType::Raise,
          "Aggressive agent must consider Raise before Check");

    checkLegalAction(action, legal);
}

void testOnlyFoldAvailable() {
    std::mt19937 rng{406};
    auto config = aggressiveConfig();
    config.bluffFrequency = 1.0;

    HeuristicAgent agent(rng, config);

    auto observation = initialObservation(406);

    auto& legal = *observation.legalActions;
    legal.canFold = true;
    legal.canCheck = false;
    legal.canCall = false;
    legal.canBet = false;
    legal.canRaise = false;

    const auto before = rng;

    checkAction(
        agent.chooseAction(observation),
        ActionType::Fold,
        "Agent must Fold when Fold is the only legal action"
    );

    check(rng == before,
          "Immediate Fold must not consume randomness");
}

void testNoActionAvailable() {
    std::mt19937 rng{407};
    HeuristicAgent agent(rng, aggressiveConfig());

    auto observation = initialObservation(407);

    auto& legal = *observation.legalActions;
    legal.canFold = false;
    legal.canCheck = false;
    legal.canCall = false;
    legal.canBet = false;
    legal.canRaise = false;

    bool threw = false;

    try {
        (void)agent.chooseAction(observation);
    } catch (const std::logic_error&) {
        threw = true;
    }

    check(threw,
          "Agent must reject a state with no legal actions");
}

// ============================================================
// H3.5 — Reproducibility and integration
// ============================================================

void testPassiveReproducibility() {
    auto observation = initialObservation(47);

    std::mt19937 rng1{12345};
    std::mt19937 rng2{12345};

    HeuristicAgent agent1(rng1);
    HeuristicAgent agent2(rng2);

    for (int i = 0; i < 20; ++i) {
        const Action a = agent1.chooseAction(observation);
        const Action b = agent2.chooseAction(observation);

        check(a.type == b.type && a.amount == b.amount,
              "Passive decisions must be reproducible");
    }

    check(rng1 == rng2,
          "Passive agents must preserve RNG synchronization");
}

void testAggressiveReproducibility() {
    auto observation = bettingObservation();
    setGuaranteedTie(observation);

    auto config = aggressiveConfig();
    config.bluffFrequency = 0.35;

    std::mt19937 rng1{54321};
    std::mt19937 rng2{54321};

    HeuristicAgent agent1(rng1, config);
    HeuristicAgent agent2(rng2, config);

    for (int i = 0; i < 100; ++i) {
        const Action a = agent1.chooseAction(observation);
        const Action b = agent2.chooseAction(observation);

        check(a.type == b.type && a.amount == b.amount,
              "Aggressive decisions must be reproducible");
    }

    check(rng1 == rng2,
          "Aggressive agents must preserve RNG synchronization");
}

void testShortAllInCall() {
    std::mt19937 deckRng{48};
    std::mt19937 agentRng{108};

    HandEngine engine{
        {75, 1000},
        HandConfig{50, 100},
        0,
        deckRng
    };

    HeuristicAgent agent(agentRng);

    auto observation = engine.observe(0);

    check(observation.legalActions.has_value() &&
          observation.legalActions->canCall,
          "Short-stacked player must be allowed to Call");

    check(observation.stacks[0] <
          observation.legalActions->toCall,
          "Fixture must represent a short all-in Call");

    setGuaranteedWin(observation);

    const Action action = agent.chooseAction(observation);

    checkAction(
        action,
        ActionType::Call,
        "Unbeatable short stack must Call all-in"
    );

    engine.applyAction(0, action);

    check(engine.state().phase() == HandPhase::Finished,
          "Short all-in Call must finish the hand");

    check(
        engine.state().player(0).stack() +
        engine.state().player(1).stack() == 1075,
        "Short all-in must conserve chips"
    );
}

void runAutonomousHands(
    HeuristicMode mode,
    std::uint32_t seed
) {
    constexpr int kHands = 40;

    std::mt19937 deckRng{seed};
    std::mt19937 heuristicRng{seed + 1};
    std::mt19937 randomRng{seed + 2};

    HeuristicConfig config;
    config.mode = mode;
    config.equitySamples = 100;
    config.bluffFrequency = 0.05;

    HeuristicAgent heuristic(heuristicRng, config);
    RandomAgent random(randomRng);

    for (int hand = 0; hand < kHands; ++hand) {
        HandEngine engine{
            {1000, 1000},
            HandConfig{50, 100},
            static_cast<std::uint8_t>(hand % 2),
            deckRng
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

            // Alternate the heuristic's seat.
            const bool heuristicTurn =
                (player == static_cast<std::uint8_t>(hand % 2));

            const Action action = heuristicTurn
                ? heuristic.chooseAction(observation)
                : random.chooseAction(observation);

            // Validate before submitting to the engine.
            checkLegalAction(
                action,
                *observation.legalActions
            );

            // The engine also validates the full game rules.
            engine.applyAction(player, action);
        }

        check(engine.state().phase() == HandPhase::Finished,
              "Every autonomous hand must finish");

        check(
            engine.state().player(0).stack() +
            engine.state().player(1).stack() == 2000,
            "Autonomous hand must conserve chips"
        );
    }
}

void testPassiveAutonomousHands() {
    runAutonomousHands(HeuristicMode::Passive, 500);
}

void testAggressiveAutonomousHands() {
    runAutonomousHands(HeuristicMode::Aggressive, 600);
}

} // namespace

int main() {
    // H3.1 — Configuration and passive policy
    testNoLegalActions();
    testInvalidConfig();
    testPassiveCheck();
    testGuaranteedCall();
    testFoldWhenPotOddsAreBad();
    testCallWhenPotOddsAreGood();
    testPassiveNoCallAvailable();
    testPassiveNeverBetsOrRaises();

    // H3.2 — Value betting
    testValueBet();
    testBetSizingBounds();

    // H3.3 — Value raising
    testValueRaise();
    testRaiseFromPlayerOne();
    testRaiseSizingBounds();

    // H3.4 — Bluffing
    testBluffBetFrequencyZero();
    testBluffBetFrequencyOne();
    testBluffRaiseFrequencyZero();
    testBluffRaiseFrequencyOne();
    testNoIllegalBluff();
    testCheckWithRaiseAvailable();
    testOnlyFoldAvailable();
    testNoActionAvailable();

    // H3.5 — Reproducibility and integration
    testPassiveReproducibility();
    testAggressiveReproducibility();
    testShortAllInCall();
    testPassiveAutonomousHands();
    testAggressiveAutonomousHands();

    std::cout << "All HeuristicAgent tests passed!\n";
    return EXIT_SUCCESS;
}
