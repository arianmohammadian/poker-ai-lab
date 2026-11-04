/**
 * @file equity_input.h
 * @brief Represents a Heads-Up Texas Hold'em equity calculation input.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/card.h"

#include <array>
#include <vector>

namespace poker {

struct EquityInput {
    std::array<Card, 2> player1;
    std::array<Card, 2> player2;
    std::vector<Card> board;
};

} // namespace poker