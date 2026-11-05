/**
 * @file hand_config.h
 * @brief Defines the configuration of a hand in the game.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>

namespace poker {

struct HandConfig {
    std::int64_t smallBlind;
    std::int64_t bigBlind;
};

} // namespace poker