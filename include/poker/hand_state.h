/**
 * @file hand_state.h
 * @brief Defines the state of a hand in the game.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/player_state.h"
#include "poker/street.h"

#include <optional>
#include <vector>
#include <array>
#include <cstdint>

namespace poker {

class HandEngine;

class HandState {
    friend class HandEngine;

    
public:
    HandState(std::array<PlayerState, 2> players, std::uint8_t button);

    [[nodiscard]] const PlayerState& player(std::uint8_t index) const;
    [[nodiscard]] const std::vector<Card>& board() const noexcept;
    [[nodiscard]] Street street() const noexcept;
    [[nodiscard]] std::uint8_t button() const noexcept;
    [[nodiscard]] std::optional<std::uint8_t> actingPlayer() const noexcept;
    [[nodiscard]] std::int64_t pot() const noexcept;

private:
    std::array<PlayerState, 2> players_;
    std::vector<Card> board_;
    Street street_;
    std::uint8_t button_;
    std::optional<std::uint8_t> actingPlayer_;
};

} // namespace poker