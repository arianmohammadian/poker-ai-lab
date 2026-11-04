/**
 * @file suit.h
 * @brief Defines the four standard playing card suits.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>

namespace poker {

enum class Suit : std::uint8_t {
    Clubs = 0,
    Diamonds,
    Hearts,
    Spades
};

} // namespace poker