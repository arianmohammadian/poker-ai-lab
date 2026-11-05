/**
 * @file player_state.h
 * @brief Defines the state of a player in the game.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/card.h"

#include <array>
#include <cstdint>

namespace poker {

class PlayerState {
public:
    PlayerState(std::int64_t initialStack, std::array<Card, 2> holeCards);

    [[nodiscard]] std::int64_t stack() const noexcept;
    [[nodiscard]] std::int64_t committedStreet() const noexcept;
    [[nodiscard]] std::int64_t committedTotal() const noexcept;
    [[nodiscard]] const std::array<Card, 2>& holeCards() const noexcept;
    [[nodiscard]] bool hasFolded() const noexcept;
    [[nodiscard]] bool isAllIn() const noexcept;

    void commitChips(std::int64_t amount);
    void fold() noexcept;
    void beginNewStreet() noexcept;
    
private:
    std::int64_t stack_;
    std::array<Card, 2> holeCards_;
    std::int64_t committedStreet_;
    std::int64_t committedTotal_;
    bool folded_;
};

} // namespace poker