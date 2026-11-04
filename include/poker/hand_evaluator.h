/**
 * @file hand_evaluator.h
 * @brief Computes the rank of a hand, given by 5 cards, in the game.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/hand_rank.h"
#include "poker/card.h"

#include <array>

namespace poker {

class HandEvaluator {
public:
    [[nodiscard]] static HandRank evaluate(const std::array<Card, 5>& cards);
};

} // namespace poker