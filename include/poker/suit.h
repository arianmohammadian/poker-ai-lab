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