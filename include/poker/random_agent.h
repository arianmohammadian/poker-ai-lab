/**
 * @file random_agent.h
 * @brief Declares a random legal-action poker agent.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/agent.h"

#include <random>

namespace poker {

class RandomAgent : public Agent {
public:
    explicit RandomAgent(std::mt19937& rng);

    [[nodiscard]] Action chooseAction(
        const PlayerObservation& observation
    ) override;

private:
    std::mt19937& rng_;
};

} // namespace poker