/**
 * @file training_runner.h
 * @brief Runs reproducible self-play training episodes for tabular agents.
 * @author Arian Mohammadian
 */

#pragma once

#include "poker/learning_agent.h"

#include <cstdint>

namespace poker {

struct TrainingConfig {
    std::uint64_t episodes{1000};
    std::uint32_t seed{42};

    std::int64_t startingStack{1000};
    std::int64_t smallBlind{50};
    std::int64_t bigBlind{100};
};

struct TrainingResult {
    std::uint64_t episodesCompleted{0};
    std::uint64_t totalDecisions{0};
};

class TrainingRunner {
public:
    [[nodiscard]] static TrainingResult run(
        LearningAgent& agent0,
        LearningAgent& agent1,
        const TrainingConfig& config
    );
};

} // namespace poker