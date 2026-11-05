/**
 * @file hand_engine.h
 * @brief Defines the engine responsible for executing a single
 *        Heads-Up No-Limit Texas Hold'em hand.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/deck.h"
#include "poker/hand_config.h"
#include "poker/hand_state.h"

#include <array>
#include <cstdint>
#include <random>

namespace poker {

class HandEngine {
public:
    HandEngine(
        std::array<std::int64_t, 2> initialStacks,
        HandConfig config,
        std::uint8_t button,
        std::mt19937& rng
    );

    [[nodiscard]] const HandState& state() const noexcept;

private:
    HandConfig config_;
    Deck deck_;
    HandState state_;
};

} // namespace poker