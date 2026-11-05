#include "poker/player_state.h"

#include <stdexcept>
#include <climits>
#include <limits>

namespace poker {

using std::invalid_argument;
using std::logic_error;

PlayerState::PlayerState(std::int64_t initialStack, std::array<Card, 2> holeCards)
    : stack_(initialStack),
      holeCards_(holeCards),
      committedStreet_(0),
      committedTotal_(0),
      folded_(false)
{
    if(initialStack < 0) throw invalid_argument("Initial Stack must be non-negative");
    if(holeCards[0] == holeCards[1]) throw invalid_argument("Hole Cards must be different");
}

std::int64_t PlayerState::stack() const noexcept {
    return stack_;
}

std::int64_t PlayerState::committedStreet() const noexcept {
    return committedStreet_;
}

std::int64_t PlayerState::committedTotal() const noexcept {
    return committedTotal_;
}

const std::array<Card, 2>& PlayerState::holeCards() const noexcept {
    return holeCards_;
}

bool PlayerState::hasFolded() const noexcept {
    return folded_;
}

bool PlayerState::isAllIn() const noexcept {
    return !folded_ && stack_ == 0;
}

void PlayerState::commitChips(std::int64_t amount) {
    if(amount <= 0) throw invalid_argument("Amount must be positive");
    if(folded_) throw logic_error("Player had folded and can't bet");
    if(stack_ < amount) throw invalid_argument("Amount exceeds available stack");
    stack_ -= amount;
    committedStreet_ += amount;
    committedTotal_ += amount;
}

void PlayerState::refundChips(std::int64_t amount) {
    if(amount <= 0) throw invalid_argument("Amount must be positive");
    if (amount > committedStreet_) throw invalid_argument("Refund exceeds current street commitment");
    stack_ += amount;
    committedStreet_ -= amount;
    committedTotal_ -= amount;
}

void PlayerState::awardChips(std::int64_t amount) {
    if(amount <= 0) throw invalid_argument("Amount must be positive");
    
    const auto maxChips = std::numeric_limits<std::int64_t>::max();
    if (amount > maxChips - stack_) throw invalid_argument("Award exceeds stack capacity");

    stack_ += amount;
}

void PlayerState::fold() noexcept {
    folded_ = true;
}

void PlayerState::beginNewStreet() noexcept {
    committedStreet_ = 0;
}

} // namespace poker