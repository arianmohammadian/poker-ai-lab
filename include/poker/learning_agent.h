/**
 * @file learning_agent.h
 * @brief Tabular reinforcement learning agent using epsilon-greedy exploration.
 * @author Arian Mohammadian
 */

#pragma once

#include "poker/agent.h"
#include "poker/q_table.h"

#include <cstdint>
#include <random>
#include <vector>

namespace poker {

struct LearningConfig {
    double epsilon{0.10};
    std::uint64_t equitySamples{100};
    std::int64_t bigBlind{100};
};

struct Experience {
    AbstractState state;
    AbstractAction action;
};

class LearningAgent final : public Agent {
public:
    LearningAgent(
        std::mt19937& rng,
        QTable& table,
        LearningConfig config = {}
    );

    [[nodiscard]] Action chooseAction(
        const PlayerObservation& observation
    ) override;

    void finishEpisode(double reward);
    void clearEpisode();

private:
    std::mt19937& rng_;
    QTable& table_;
    LearningConfig config_;

    std::vector<Experience> experiences_;

    [[nodiscard]] AbstractAction selectAction(
        const AbstractState& state,
        const std::vector<AbstractAction>& legal
    );
};

} // namespace poker