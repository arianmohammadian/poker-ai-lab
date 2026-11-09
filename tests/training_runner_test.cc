/**
 * @file training_runner_test.cc
 * @brief Tests deterministic multi-episode tabular self-play training.
 * @author Arian Mohammadian
 */

#include "poker/training_runner.h"

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

void testZeroEpisodes() {
    QTable table;
    std::mt19937 rng0{1}, rng1{2};
    LearningAgent agent0(rng0, table), agent1(rng1, table);
    TrainingConfig cfg;
    cfg.episodes = 0;
    const auto result = TrainingRunner::run(agent0, agent1, cfg);
    check(result.episodesCompleted == 0 && result.totalDecisions == 0,
          "Zero episodes must perform no work");
}

void testTrainingAndReproducibility() {
    QTable table1, table2;
    std::mt19937 rng01{101}, rng11{102};
    std::mt19937 rng02{101}, rng12{102};
    LearningConfig lc;
    lc.epsilon = 0.2;
    lc.equitySamples = 10;
    lc.bigBlind = 100;

    LearningAgent a1(rng01, table1, lc), b1(rng11, table1, lc);
    LearningAgent a2(rng02, table2, lc), b2(rng12, table2, lc);

    TrainingConfig cfg;
    cfg.episodes = 4;
    cfg.seed = 123;
    const auto r1 = TrainingRunner::run(a1, b1, cfg);
    const auto r2 = TrainingRunner::run(a2, b2, cfg);

    check(r1.episodesCompleted == 4 && r2.episodesCompleted == 4,
          "All configured episodes must finish");
    check(r1.totalDecisions > 0 && r1.totalDecisions == r2.totalDecisions,
          "Decision totals must be positive and reproducible");
    check(rng01 == rng02 && rng11 == rng12,
          "Agent RNG states must be reproducible");

    std::uint64_t updates = 0;
    const AbstractAction actions[] = {
        AbstractAction::Fold, AbstractAction::CheckCall,
        AbstractAction::HalfPot, AbstractAction::Pot, AbstractAction::AllIn
    };
    const Street streets[] = {
        Street::Preflop, Street::Flop, Street::Turn, Street::River
    };

    for (Street street : streets)
        for (std::uint8_t equity = 0; equity < 5; ++equity)
            for (std::uint8_t stack = 0; stack < 4; ++stack)
                for (std::uint8_t odds = 0; odds < 4; ++odds)
                    for (bool button : {false, true}) {
                        const AbstractState state{street, equity, stack, odds, button};
                        for (AbstractAction action : actions) {
                            const auto visits = table1.visits(state, action);
                            updates += visits;
                            check(visits == table2.visits(state, action) &&
                                  table1.value(state, action) == table2.value(state, action),
                                  "Two seeded trainings must produce identical Q-tables");
                        }
                    }

    check(updates == r1.totalDecisions,
          "Each decision must produce exactly one Q update");
}

void testInvalidConfig() {
    QTable table;
    std::mt19937 rng0{1}, rng1{2};
    LearningAgent a(rng0, table), b(rng1, table);
    TrainingConfig cfg;
    cfg.bigBlind = 0;
    bool rejected = false;
    try { [[maybe_unused]] auto result = TrainingRunner::run(a, b, cfg); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Zero big blind must be rejected");
}

} // namespace

int main() {
    testZeroEpisodes();
    testTrainingAndReproducibility();
    testInvalidConfig();
    std::cout << "All TrainingRunner tests passed!\n";
    return EXIT_SUCCESS;
}
