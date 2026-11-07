
/**
 * @file heuristic_agent.h
 * @brief Equity-based heuristic agent for Heads-Up NLHE.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/agent.h"

#include <cstdint>
#include <random>

namespace poker {

class HeuristicAgent final : public Agent {
public:
    explicit HeuristicAgent(std::mt19937& rng);

    [[nodiscard]] Action chooseAction(
        const PlayerObservation& observation
    ) override;

private:
    std::mt19937& rng_;

    static constexpr std::uint64_t kEquitySamples = 1000;
};

} // namespace poker
