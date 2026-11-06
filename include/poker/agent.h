/**
 * @file agent.h
 * @brief Defines the interface for poker agents.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/action.h"
#include "poker/player_observation.h"

namespace poker {

class Agent {
public:
    virtual ~Agent() = default;

    [[nodiscard]] virtual Action chooseAction(
        const PlayerObservation& observation
    ) = 0;
};

} // namespace poker