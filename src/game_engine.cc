/**
 * @file game_engine.cc
 * @brief Implements Heads-Up NLHE game session management.
 * @author Arian Mohammadian
 */

#include "poker/game_engine.h"

#include <stdexcept>

namespace poker {

GameEngine::GameEngine(
    std::array<std::int64_t, 2> initialStacks,
    HandConfig config,
    std::uint8_t button,
    std::mt19937& rng
)
    : config_(config),
      button_(button),
      handNumber_(1),
      rng_(rng)
{
    hand_.emplace(initialStacks, config, button, rng);
}

const HandEngine& GameEngine::currentHand() const noexcept {
    return *hand_;
}

bool GameEngine::isFinished() const noexcept {
    const HandState& state = hand_->state();

    return state.phase() == HandPhase::Finished &&
           (state.player(0).stack() == 0 ||
            state.player(1).stack() == 0);
}

std::uint64_t GameEngine::handNumber() const noexcept {
    return handNumber_;
}

void GameEngine::applyAction(std::uint8_t player, Action action) {
    hand_->applyAction(player, action);
}

void GameEngine::nextHand() {
    if (hand_->state().phase() != HandPhase::Finished)
        throw std::logic_error("Current hand has not finished");

    if (isFinished())
        throw std::logic_error("Game has finished");

    const HandState& state = hand_->state();

    const std::array<std::int64_t, 2> nextStacks{
        state.player(0).stack(),
        state.player(1).stack()
    };

    const std::uint8_t nextButton = 1 - button_;

    hand_.emplace(nextStacks, config_, nextButton, rng_);

    button_ = nextButton;
    ++handNumber_;
}

} // namespace poker