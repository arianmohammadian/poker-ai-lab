/**
 * @file abstract_action.h
 * @brief Defines abstract betting actions for tabular poker agents.
 * @author Arian Mohammadian
 */

#pragma once

#include "poker/action.h"
#include "poker/player_observation.h"

#include <array>
#include <cstdint>
#include <vector>

namespace poker {

enum class AbstractAction : std::uint8_t {
    Fold,
    CheckCall,
    HalfPot,
    Pot,
    AllIn
};

class ActionAbstractor {
public:
    [[nodiscard]] static std::vector<AbstractAction> legalActions(
        const PlayerObservation& observation
    );

    [[nodiscard]] static Action toConcrete(
        AbstractAction action,
        const PlayerObservation& observation
    );
};

} // namespace poker