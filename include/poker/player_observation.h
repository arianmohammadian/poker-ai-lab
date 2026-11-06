/**
 * @file player_observation.h
 * @brief Defines the information visible to a player.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/card.h"
#include "poker/street.h"
#include "poker/legal_actions.h"

#include <optional>
#include <array>
#include <vector>
#include <cstdint>

namespace poker {

struct PlayerObservation {
    std::uint8_t player;
    std::array<Card, 2> holeCards;
    std::vector<Card> board;
    std::array<std::int64_t, 2> stacks;
    std::array<std::int64_t, 2> committedStreet;
    std::int64_t pot;
    Street street;
    std::uint8_t button;
    std::optional<LegalActions> legalActions;
};

} // namespace poker