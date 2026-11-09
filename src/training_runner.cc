/**
 * @file training_runner.cc
 * @brief Implements self-play training over independent poker hands.
 * @author Arian Mohammadian
 */

#include "poker/training_runner.h"
#include "poker/self_play_runner.h"

#include <random>
#include <stdexcept>

namespace poker {

TrainingResult TrainingRunner::run(
    LearningAgent& agent0,
    LearningAgent& agent1,
    const TrainingConfig& config
) {
    // TODO 1: Validate configuration.
    if(config.smallBlind < 0 || config.bigBlind < 0 ||
    config.smallBlind >= config.bigBlind) throw std::invalid_argument("Invalid blinds");

    if(config.startingStack < 0) throw std::invalid_argument("Invalid starting stack");

    TrainingResult result{};

    std::mt19937 episodeRng(config.seed);

    agent0.clearEpisode();
    agent1.clearEpisode();

    for (std::uint64_t i = 0; i < config.episodes; ++i) {
        // TODO 2: Alternate button between seats 0 and 1.
        const std::uint8_t button = static_cast<std::uint8_t>(i % 2);

        // TODO 3: Execute a hand with SelfPlayRunner.
        // Use episodeRng() as the deck seed.
        EpisodeResult episode = SelfPlayRunner::runHand(
            agent0,
            agent1,
            {config.startingStack, config.startingStack},
            {config.smallBlind, config.bigBlind},
            button,
            episodeRng()
        );

        // TODO 4: Normalize both rewards to big blinds.
        const double reward0 = static_cast<double>(episode.rewards[0]) / config.bigBlind;
        const double reward1 = static_cast<double>(episode.rewards[1]) / config.bigBlind;

        // TODO 5: Call finishEpisode() for both agents.
        agent0.finishEpisode(reward0);
        agent1.finishEpisode(reward1);

        // TODO 6: Update training statistics.
        ++result.episodesCompleted;
        result.totalDecisions += episode.decisions;
    }

    return result;
}

} // namespace poker