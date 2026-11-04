/**
 * @file hand_rank.h
 * @brief Defines the rank of a hand in the game.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/hand_category.h"

#include <array>
#include <cstdint>
#include <compare>

namespace poker {

class HandRank {
public:
    using Kickers = std::array<std::uint8_t, 5>;

    HandRank(HandCategory category, Kickers kickers);

    [[nodiscard]] HandCategory category() const noexcept;
    [[nodiscard]] const Kickers& kickers() const noexcept;

    auto operator<=>(const HandRank&) const = default;
    bool operator==(const HandRank&) const = default;

private:
    HandCategory category_;
    Kickers kickers_;
};

} // namespace poker