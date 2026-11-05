#include "poker/hand_state.h"

#include <stdexcept>
#include <utility>
#include <set>
#include <limits>

namespace poker {

using std::invalid_argument;

namespace {

int cardId(const Card& card) {
    int rank = static_cast<int>(card.rank());
    int suit = static_cast<int>(card.suit());

    return (rank - 2) * 4 + suit;
}

} // namespace

HandState::HandState(std::array<PlayerState, 2> players, std::uint8_t button)
    : players_(std::move(players)),
      board_(),
      street_(Street::Preflop),
      button_(button),
      actingPlayer_(std::nullopt),
      phase_(HandPhase::Setup),
      actionsToClose_(0),
      lastFullRaise_(0),
      payouts_{0, 0}
{
    if(button > 1) throw invalid_argument("Button must be 0 or 1");

    std::set<int> seenCards;

    for(const Card& c : players_[0].holeCards()) {
        seenCards.insert(cardId(c));
    }

    for(const Card& c : players_[1].holeCards()) {
        seenCards.insert(cardId(c));
    }

    if(seenCards.size() != 4) throw invalid_argument("There are duplicated cards");

    if (players_[0].committedTotal() != 0 ||
        players_[1].committedTotal() != 0) {
        throw invalid_argument("Players must have no committed chips");
    }

    if(players_[0].hasFolded() || players_[1].hasFolded()) {
        throw invalid_argument("Players must not have folded");
    }

    const auto maxChips = std::numeric_limits<std::int64_t>::max();

    if (players_[0].stack() > maxChips - players_[1].stack()) {
        throw invalid_argument("Total chips exceed int64_t capacity");
    }
}

const PlayerState& HandState::player(std::uint8_t index) const {
    return players_.at(index);
}

const std::vector<Card>& HandState::board() const noexcept {
    return board_;
}

Street HandState::street() const noexcept {
    return street_;
}

std::uint8_t HandState::button() const noexcept {
    return button_;
}

std::optional<std::uint8_t> HandState::actingPlayer() const noexcept {
    return actingPlayer_;
}

std::int64_t HandState::pot() const noexcept {
    return players_[0].committedTotal() + players_[1].committedTotal()
    - payouts_[0] - payouts_[1];
}

std::int64_t HandState::payout(std::uint8_t index) const {
    return payouts_.at(index);
}

HandPhase HandState::phase() const noexcept {
    return phase_;
}

std::uint8_t HandState::actionsToClose() const noexcept {
    return actionsToClose_;
}

std::int64_t HandState::lastFullRaise() const noexcept {
    return lastFullRaise_;
}

} // namespace poker