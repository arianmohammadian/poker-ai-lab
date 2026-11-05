/**
 * @file hand_phase.h
 * @brief Defines the lifecycle phases of a poker hand.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>

namespace poker {

enum class HandPhase : std::uint8_t {
    Setup,
    Betting,
    Finished
};

} // namespace poker