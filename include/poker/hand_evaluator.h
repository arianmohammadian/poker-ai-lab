/**
 * @file hand_evaluator.h
 * @brief Evaluates five-card and seven-card poker hands.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/hand_rank.h"
#include "poker/card.h"

#include <array>

namespace poker {

class HandEvaluator {
public:
    [[nodiscard]] static HandRank evaluate(const std::array<Card, 7>& cards);
    [[nodiscard]] static HandRank evaluate(const std::array<Card, 5>& cards);
};

} // namespace poker