/**
 * @file hand_category.h
 * @brief Defines the nine hand categories (A Royal Flush is considered a Straight Flush with top card: Ace).
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>

namespace poker {

enum class HandCategory : std::uint8_t {
    HighCard = 0,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush
};

} // namespace poker