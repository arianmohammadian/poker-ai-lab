
/**
 * @file match_runner_test.cc
 * @brief Tests for reproducible AI match benchmarking.
 * @author Arian Mohammadian
 */

#include "poker/match_runner.h"
#include "poker/random_agent.h"
#include "poker/heuristic_agent.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>

using poker::HeuristicAgent;
using poker::HeuristicMode;
using poker::MatchConfig;
using poker::MatchResult;
using poker::MatchRunner;
using poker::RandomAgent;

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void checkResultInvariants(
    const MatchResult& result,
    std::uint64_t expectedGames
) {
    check(
        result.gamesPlayed == expectedGames,
        "Incorrect number of completed games"
    );

    check(
        result.winsA + result.winsB + result.ties ==
            result.gamesPlayed,
        "Game outcomes must sum to gamesPlayed"
    );

    check(
        result.totalHands >= result.gamesPlayed,
        "Every completed game must contain at least one hand"
    );
}

void checkSameResult(
    const MatchResult& a,
    const MatchResult& b
) {
    check(a.gamesPlayed == b.gamesPlayed,
          "Reproducibility: gamesPlayed differs");

    check(a.winsA == b.winsA,
          "Reproducibility: winsA differs");

    check(a.winsB == b.winsB,
          "Reproducibility: winsB differs");

    check(a.ties == b.ties,
          "Reproducibility: ties differs");

    check(a.totalHands == b.totalHands,
          "Reproducibility: totalHands differs");
}

poker::HeuristicConfig fastConfig(HeuristicMode mode) {
    poker::HeuristicConfig config;
    config.mode = mode;
    config.equitySamples = 50;
    return config;
}

// ------------------------------------------------------------
// Test 1: A single game must finish correctly.
// ------------------------------------------------------------
void testSingleGame() {
    std::mt19937 rngA{101};
    std::mt19937 rngB{102};

    RandomAgent agentA(rngA);
    RandomAgent agentB(rngB);

    MatchConfig config;
    config.games = 1;

    const MatchResult result =
        MatchRunner::run(agentA, agentB, config);

    checkResultInvariants(result, 1);
}

// ------------------------------------------------------------
// Test 2: Multiple independent games must complete.
// ------------------------------------------------------------
void testMultipleGames() {
    std::mt19937 rngA{201};
    std::mt19937 rngB{202};

    RandomAgent agentA(rngA);
    RandomAgent agentB(rngB);

    MatchConfig config;
    config.games = 10;

    const MatchResult result =
        MatchRunner::run(agentA, agentB, config);

    checkResultInvariants(result, 10);
}

// ------------------------------------------------------------
// Test 3: Identical seeds must produce identical results.
// ------------------------------------------------------------
void testReproducibility() {
    std::mt19937 rngA1{301};
    std::mt19937 rngB1{302};

    std::mt19937 rngA2{301};
    std::mt19937 rngB2{302};

    RandomAgent agentA1(rngA1);
    RandomAgent agentB1(rngB1);

    RandomAgent agentA2(rngA2);
    RandomAgent agentB2(rngB2);

    MatchConfig config;
    config.games = 5;
    config.seed = 42;

    const MatchResult first =
        MatchRunner::run(agentA1, agentB1, config);

    const MatchResult second =
        MatchRunner::run(agentA2, agentB2, config);

    checkResultInvariants(first, 5);
    checkResultInvariants(second, 5);

    checkSameResult(first, second);

    check(rngA1 == rngA2,
          "Agent A RNG states must match");

    check(rngB1 == rngB2,
          "Agent B RNG states must match");
}

// ------------------------------------------------------------
// Test 4: Zero games must produce an empty result.
// ------------------------------------------------------------
void testZeroGames() {
    std::mt19937 rngA{401};
    std::mt19937 rngB{402};

    RandomAgent agentA(rngA);
    RandomAgent agentB(rngB);

    MatchConfig config;
    config.games = 0;

    const auto beforeA = rngA;
    const auto beforeB = rngB;

    const MatchResult result =
        MatchRunner::run(agentA, agentB, config);

    check(result.gamesPlayed == 0,
          "Zero games must produce zero completed games");

    check(result.winsA == 0 && result.winsB == 0,
          "Zero games must produce zero wins");

    check(result.ties == 0,
          "Zero games must produce zero ties");

    check(result.totalHands == 0,
          "Zero games must produce zero hands");

    check(rngA == beforeA && rngB == beforeB,
          "Zero games must not consume agent randomness");
}

// ------------------------------------------------------------
// Test 5: The passive heuristic must complete real games.
// ------------------------------------------------------------
void testPassiveVsRandom() {
    std::mt19937 heuristicRng{501};
    std::mt19937 randomRng{502};

    HeuristicAgent heuristic(
        heuristicRng,
        fastConfig(HeuristicMode::Passive)
    );

    RandomAgent random(randomRng);

    MatchConfig config;
    config.games = 2;
    config.seed = 503;

    const MatchResult result =
        MatchRunner::run(heuristic, random, config);

    checkResultInvariants(result, 2);
}

// ------------------------------------------------------------
// Test 6: The aggressive heuristic must complete real games.
// ------------------------------------------------------------
void testAggressiveVsRandom() {
    std::mt19937 heuristicRng{601};
    std::mt19937 randomRng{602};

    HeuristicAgent heuristic(
        heuristicRng,
        fastConfig(HeuristicMode::Aggressive)
    );

    RandomAgent random(randomRng);

    MatchConfig config;
    config.games = 2;
    config.seed = 603;

    const MatchResult result =
        MatchRunner::run(heuristic, random, config);

    checkResultInvariants(result, 2);
}

// ------------------------------------------------------------
// Test 7: Passive and aggressive agents must play each other.
// ------------------------------------------------------------
void testPassiveVsAggressive() {
    std::mt19937 passiveRng{701};
    std::mt19937 aggressiveRng{702};

    HeuristicAgent passive(
        passiveRng,
        fastConfig(HeuristicMode::Passive)
    );

    HeuristicAgent aggressive(
        aggressiveRng,
        fastConfig(HeuristicMode::Aggressive)
    );

    MatchConfig config;
    config.games = 2;
    config.seed = 703;

    const MatchResult result =
        MatchRunner::run(passive, aggressive, config);

    checkResultInvariants(result, 2);
}

// ------------------------------------------------------------
// Test 8: Aggressive heuristic benchmarks must be reproducible.
// ------------------------------------------------------------
void testHeuristicReproducibility() {
    std::mt19937 rngA1{801};
    std::mt19937 rngB1{802};

    std::mt19937 rngA2{801};
    std::mt19937 rngB2{802};

    HeuristicAgent agentA1(
    rngA1, fastConfig(HeuristicMode::Aggressive)
    );
    HeuristicAgent agentB1(
        rngB1, fastConfig(HeuristicMode::Passive)
    );

    HeuristicAgent agentA2(
        rngA2, fastConfig(HeuristicMode::Aggressive)
    );
    HeuristicAgent agentB2(
        rngB2, fastConfig(HeuristicMode::Passive)
    );

    MatchConfig config;
    config.games = 2;
    config.seed = 803;

    const MatchResult first =
        MatchRunner::run(agentA1, agentB1, config);

    const MatchResult second =
        MatchRunner::run(agentA2, agentB2, config);

    checkResultInvariants(first, 2);
    checkSameResult(first, second);

    check(rngA1 == rngA2,
          "Aggressive RNG states must match");

    check(rngB1 == rngB2,
          "Passive RNG states must match");
}

} // namespace

int main() {
    testSingleGame();
    testMultipleGames();
    testReproducibility();
    testZeroGames();

    testPassiveVsRandom();
    testAggressiveVsRandom();
    testPassiveVsAggressive();
    testHeuristicReproducibility();

    std::cout << "All MatchRunner tests passed!\n";

    return EXIT_SUCCESS;
}
