/**
 * @file street.h
 * @brief Defines the four standard poker game streets.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>

namespace poker {

enum class Street : std::uint8_t {
    Preflop,
    Flop, 
    Turn, 
    River
};

} // namespace poker