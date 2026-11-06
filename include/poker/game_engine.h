/**
 * @file game_engine.h
 * @brief Manages consecutive Heads-Up NLHE hands.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/hand_engine.h"

#include <array>
#include <cstdint>
#include <optional>
#include <random>

namespace poker {

class GameEngine {
public:
    GameEngine(
        std::array<std::int64_t, 2> initialStacks,
        HandConfig config,
        std::uint8_t button,
        std::mt19937& rng
    );

    [[nodiscard]] const HandEngine& currentHand() const noexcept;

    [[nodiscard]] bool isFinished() const noexcept;

    [[nodiscard]] std::uint64_t handNumber() const noexcept;

    void applyAction(std::uint8_t player, Action action);

    void nextHand();

private:
    HandConfig config_;
    std::uint8_t button_;
    std::uint64_t handNumber_;
    std::mt19937& rng_;
    std::optional<HandEngine> hand_;
};

} // namespace poker