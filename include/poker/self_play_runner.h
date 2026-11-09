/**
 * @file self_play_runner.h
 * @brief Runs individual poker hands between two agents for self-play training.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/agent.h"
#include "poker/hand_config.h"

#include <array>
#include <cstdint>

namespace poker {

struct EpisodeResult {
    std::array<std::int64_t, 2> initialStacks;
    std::array<std::int64_t, 2> finalStacks;
    std::array<std::int64_t, 2> rewards;
    std::uint64_t decisions{0};
};

class SelfPlayRunner {
public:
    [[nodiscard]] static EpisodeResult runHand(
        Agent& agent0,
        Agent& agent1,
        std::array<std::int64_t, 2> stacks,
        HandConfig config,
        std::uint8_t button,
        std::uint32_t seed
    );
};

} // namespace poker