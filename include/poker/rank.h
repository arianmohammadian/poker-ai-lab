/**
 * @file rank.h
 * @brief Defines the thirteen standard playing card ranks.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>

namespace poker {

enum class Rank : std::uint8_t {
    Two = 2,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace
};

} // namespace poker