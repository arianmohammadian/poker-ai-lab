
/**
 * @file benchmark_main.cc
 * @brief Interactive CLI for benchmarking poker agents.
 * @author Arian Mohammadian
 */

#include "poker/match_runner.h"
#include "poker/heuristic_agent.h"
#include "poker/random_agent.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

namespace {

enum class AgentChoice {
    Random = 1,
    Passive = 2,
    Aggressive = 3
};

AgentChoice readAgentChoice(const std::string& name) {
    int choice = 0;

    while (true) {
        std::cout
            << "\nChoose " << name << ":\n"
            << "  1. RandomAgent\n"
            << "  2. HeuristicAgent Passive\n"
            << "  3. HeuristicAgent Aggressive\n"
            << "Option: ";

        if (std::cin >> choice && choice >= 1 && choice <= 3) {
            return static_cast<AgentChoice>(choice);
        }

        if (std::cin.eof()) {
            throw std::runtime_error("Input ended");
        }

        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        std::cout << "Invalid option. Please enter 1, 2 or 3.\n";
    }
}

std::uint64_t readGames() {
    std::uint64_t games = 0;

    while (true) {
        std::cout << "\nNumber of games: ";

        if (std::cin >> games && games > 0) {
            return games;
        }

        if (std::cin.eof()) {
            throw std::runtime_error("Input ended");
        }

        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        std::cout << "Please enter a positive integer.\n";
    }
}

std::uint32_t readSeed() {
    std::uint64_t seed = 0;

    while (true) {
        std::cout << "Deck seed (0 to 4294967295): ";

        if (std::cin >> seed &&
            seed <= std::numeric_limits<std::uint32_t>::max()) {
            return static_cast<std::uint32_t>(seed);
        }

        if (std::cin.eof()) {
            throw std::runtime_error("Input ended");
        }

        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        std::cout << "Invalid seed.\n";
    }
}

std::unique_ptr<poker::Agent> createAgent(
    AgentChoice choice,
    std::mt19937& rng
) {
    switch (choice) {
        case AgentChoice::Random:
            return std::make_unique<poker::RandomAgent>(rng);

        case AgentChoice::Passive:
            return std::make_unique<poker::HeuristicAgent>(
                rng, poker::HeuristicMode::Passive
            );

        case AgentChoice::Aggressive:
            return std::make_unique<poker::HeuristicAgent>(
                rng, poker::HeuristicMode::Aggressive
            );
    }

    throw std::invalid_argument("Unknown agent choice");
}

std::string agentName(AgentChoice choice) {
    switch (choice) {
        case AgentChoice::Random:
            return "RandomAgent";

        case AgentChoice::Passive:
            return "HeuristicAgent Passive";

        case AgentChoice::Aggressive:
            return "HeuristicAgent Aggressive";
    }

    return "Unknown";
}

void printResults(
    const poker::MatchResult& result,
    const poker::MatchConfig& config,
    AgentChoice choiceA,
    AgentChoice choiceB
) {
    const double games = static_cast<double>(result.gamesPlayed);

    const double winRateA = games > 0
        ? 100.0 * result.winsA / games : 0.0;

    const double winRateB = games > 0
        ? 100.0 * result.winsB / games : 0.0;

    const double tieRate = games > 0
        ? 100.0 * result.ties / games : 0.0;

    const double avgHands = games > 0
        ? static_cast<double>(result.totalHands) / games : 0.0;

    std::cout << std::fixed << std::setprecision(2);

    std::cout
        << "\n========================================\n"
        << "       POKER AI LAB - BENCHMARK\n"
        << "========================================\n\n"

        << "Agent A:       " << agentName(choiceA) << '\n'
        << "Agent B:       " << agentName(choiceB) << '\n'
        << "Games:         " << config.games << '\n'
        << "Initial stack: " << config.startingStack << '\n'
        << "Blinds:        " << config.smallBlind
        << " / " << config.bigBlind << '\n'
        << "Deck seed:     " << config.seed << '\n'

        << "\n--------------- RESULTS ----------------\n\n"

        << "Agent A (" << agentName(choiceA) << ") wins:  " << result.winsA
        << " (" << winRateA << "%)\n"

        << "Agent B (" << agentName(choiceB) << ") wins:  " << result.winsB
        << " (" << winRateB << "%)\n"

        << "Ties:          " << result.ties
        << " (" << tieRate << "%)\n"

        << "\nTotal games:   " << result.gamesPlayed
        << "\nTotal hands:   " << result.totalHands
        << "\nHands/game:    " << avgHands

        << "\n\n========================================\n";
}

} // namespace

int main() {
    try {
        std::cout
            << "\n========================================\n"
            << "       POKER AI LAB - BENCHMARK\n"
            << "========================================\n";

        const AgentChoice choiceA = readAgentChoice("Agent A");
        const AgentChoice choiceB = readAgentChoice("Agent B");

        poker::MatchConfig config;
        config.games = readGames();
        config.seed = readSeed();

        // Independent and reproducible agent RNGs.
        std::mt19937 rngA{1234};
        std::mt19937 rngB{5678};

        auto agentA = createAgent(choiceA, rngA);
        auto agentB = createAgent(choiceB, rngB);

        std::cout << "\nRunning benchmark...\n";

        const poker::MatchResult result =
            poker::MatchRunner::run(*agentA, *agentB, config);

        printResults(result, config, choiceA, choiceB);

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Benchmark error: " << e.what() << '\n';
        return 1;
    }
}
