#include "poker/hand_engine.h"

#include <stdexcept>
#include <utility>
#include <limits>

namespace poker {

using std::invalid_argument;

namespace {

HandConfig validateConfigAndInputs(
    HandConfig config,
    const std::array<std::int64_t, 2>& initialStacks,
    std::uint8_t button
) {
    if (config.smallBlind <= 0 || config.bigBlind <= config.smallBlind) {
        throw std::invalid_argument("Blinds must satisfy 0 < SB < BB");
    }

    if (button > 1) {
        throw std::invalid_argument("Button must be 0 or 1");
    }

    if (initialStacks[0] <= 0 || initialStacks[1] <= 0) {
        throw std::invalid_argument("Initial stacks must be positive");
    }

    const auto maxChips = std::numeric_limits<std::int64_t>::max();

    if (initialStacks[0] > maxChips - initialStacks[1]) {
        throw std::invalid_argument("Total chips exceed int64_t capacity");
    }

    return config;
}

HandState createInitialState(
    Deck& deck,
    std::array<std::int64_t, 2> initialStacks,
    std::uint8_t button
) {
    Card c1 = deck.draw();
    Card c2 = deck.draw();
    Card c3 = deck.draw();
    Card c4 = deck.draw();

    std::array<Card, 2> holeCards0 =
        button == 0 ? std::array<Card, 2>{c1, c3}
                    : std::array<Card, 2>{c2, c4};

    std::array<Card, 2> holeCards1 =
        button == 0 ? std::array<Card, 2>{c2, c4}
                    : std::array<Card, 2>{c1, c3};

    return HandState{
        {
            PlayerState{initialStacks[0], holeCards0},
            PlayerState{initialStacks[1], holeCards1}
        },
        button
    };
}

Deck createShuffledDeck(std::mt19937& rng) {
    Deck deck;
    deck.shuffle(rng);

    return deck;
}


} // namespace

HandEngine::HandEngine(
    std::array<std::int64_t, 2> initialStacks,
    HandConfig config,
    std::uint8_t button,
    std::mt19937& rng
)
    : config_(validateConfigAndInputs(config, initialStacks, button)),
      deck_(createShuffledDeck(rng)),
      state_(createInitialState(deck_, initialStacks, button))
{
    postBlinds();
    initializePreflop();
}

const HandState& HandEngine::state() const noexcept {
    return state_;
}

void HandEngine::postBlinds() {
    const auto sb = state_.button_;
    const auto bb = 1 - sb;

    PlayerState& smallBlindPlayer = state_.players_[sb];
    PlayerState& bigBlindPlayer = state_.players_[bb];

    const auto sbAmount =
        std::min(config_.smallBlind, smallBlindPlayer.stack());

    const auto bbAmount =
        std::min(config_.bigBlind, bigBlindPlayer.stack());

    smallBlindPlayer.commitChips(sbAmount);
    bigBlindPlayer.commitChips(bbAmount);
}

void HandEngine::applyAction(std::uint8_t player, Action action) {
    if (state_.phase_ != HandPhase::Betting) {
        throw std::logic_error("Hand is not accepting actions");
    }

    if (!state_.actingPlayer_.has_value() ||
        state_.actingPlayer_.value() != player) {
        throw std::invalid_argument("It is not this player's turn");
    }

    if (action.amount != 0) {
        throw std::invalid_argument(
            "Fold and Call actions must have amount zero"
        );
    }

    PlayerState& current = state_.players_[player];
    PlayerState& opponent = state_.players_[1 - player];

    if (current.hasFolded() || current.isAllIn()) {
        throw std::logic_error("Player cannot act");
    }

    switch (action.type) {
        case ActionType::Fold: {
            current.fold();
            settleFold();
            return;
        }

        case ActionType::Call: {
            const std::int64_t toCall =
                opponent.committedStreet() -
                current.committedStreet();

            if (toCall <= 0) {
                throw std::invalid_argument(
                    "No outstanding bet to call"
                );
            }

            if (toCall > current.stack()) {
                throw std::logic_error(
                    "Insufficient stack for this Call"
                );
            }

            if (state_.actionsToClose_ == 0) {
                throw std::logic_error(
                    "No betting actions should be pending"
                );
            }

            current.commitChips(toCall);

            --state_.actionsToClose_;

            if (opponent.isAllIn()) {
                state_.actionsToClose_ = 0;
                state_.actingPlayer_ = std::nullopt;
                resolveAllIn();
                return;
            }

            if (state_.actionsToClose_ == 0) {
                state_.actingPlayer_ = std::nullopt;

                // TODO: Advance to the next street.
                // Not implemented yet.
            } else {
                state_.actingPlayer_ = 1 - player;
            }

            return;
        }

        default:
            throw std::invalid_argument(
                "Action type is not implemented yet"
            );
    }
}

void HandEngine::initializePreflop() {
    const auto sb = state_.button_;
    const auto bb = 1 - sb;

    PlayerState& smallBlindPlayer = state_.players_[sb];
    PlayerState& bigBlindPlayer = state_.players_[bb];

    state_.lastFullRaise_ = config_.bigBlind;

    // Case 1: Neither player is all-in.
    if (!smallBlindPlayer.isAllIn() && !bigBlindPlayer.isAllIn()) {
        state_.phase_ = HandPhase::Betting;
        state_.actingPlayer_ = sb;
        state_.actionsToClose_ = 2;
        return;
    }

    if (smallBlindPlayer.isAllIn()) {
        state_.actionsToClose_ = 0;
        state_.actingPlayer_ = std::nullopt;
        resolveAllIn();
        return;
    }

    // Case 3: BB is all-in.
    if (bigBlindPlayer.isAllIn()) {

        const auto sbCommit = smallBlindPlayer.committedStreet();
        const auto bbCommit = bigBlindPlayer.committedStreet();

        if (bbCommit > sbCommit) {
            // TODO: Set Betting phase.
            // TODO: Assign SB as acting player.
            // TODO: Set actionsToClose_ to 1.
            state_.phase_ = HandPhase::Betting;
            state_.actingPlayer_ = sb;
            state_.actionsToClose_ = 1;
        } else {
            state_.actionsToClose_ = 0;
            state_.actingPlayer_ = std::nullopt;
            resolveAllIn();
        }

        return;
    }
}

void HandEngine::runoutBoard() {
    const auto size = state_.board_.size();

    if (size != 0 && size != 3 && size != 4 && size != 5) {
        throw std::logic_error("Invalid board size for runout");
    }

    if(state_.board_.size() == 0) {
        (void)deck_.draw();
        for(int i = 0; i < 3; i++) state_.board_.push_back(deck_.draw());
    } 
    
    if(state_.board_.size() == 3) {
        (void)deck_.draw();
        state_.board_.push_back(deck_.draw());
    } 
    
    if(state_.board_.size() == 4) {
        (void)deck_.draw();
        state_.board_.push_back(deck_.draw());
    }
}

void HandEngine::settleShowdown() {
    if(state_.board_.size() != 5) throw invalid_argument("Board must have 5 cards");
    if(state_.players_[0].hasFolded() || state_.players_[1].hasFolded()) throw invalid_argument("Both players must not have folded");

    const auto& hole0 = state_.players_[0].holeCards(), hole1 = state_.players_[1].holeCards();
    const auto& board = state_.board_;

    std::array<Card, 7> cardsp0 = {
        hole0[0], hole0[1],
        board[0], board[1], board[2], board[3], board[4]
    };

    std::array<Card, 7> cardsp1 = {
        hole1[0], hole1[1],
        board[0], board[1], board[2], board[3], board[4]
    };

    HandRank rankp0 = HandEvaluator::evaluate(cardsp0);
    HandRank rankp1 = HandEvaluator::evaluate(cardsp1); 

    int winner = -1;

    if (rankp0 > rankp1) winner = 0;
    else if (rankp1 > rankp0) winner = 1;

    // Prevent double settlement
    if (state_.phase_ == HandPhase::Finished || state_.pot() == 0) {
        throw std::logic_error("Hand is already settled or has no pot");
    }

    const std::int64_t pot = state_.pot();
    std::array<std::int64_t, 2> payouts{0, 0};

    // Calculate payouts
    if (winner == -1) {
        // Split pot
        payouts[0] = pot / 2;
        payouts[1] = pot / 2;

        // Odd chip goes to BB (player opposite the button)
        payouts[1 - state_.button_] += pot % 2;
    } else {
        payouts[winner] = pot;
    }

    // Award chips
    for (int i = 0; i < 2; ++i) {
        if (payouts[i] > 0) {
            state_.players_[i].awardChips(payouts[i]);
        }
    }

    // Record payouts
    state_.payouts_ = payouts;

    // Mark hand as finished
    state_.phase_ = HandPhase::Finished;
    state_.actingPlayer_ = std::nullopt;
    state_.actionsToClose_ = 0;
}

void HandEngine::resolveAllIn() {
    const auto sb = state_.button_;
    const auto bb = 1 - sb;

    PlayerState& smallBlindPlayer = state_.players_[sb];
    PlayerState& bigBlindPlayer = state_.players_[bb];

    if(state_.phase() == HandPhase::Finished) throw invalid_argument("Hand must not be finished");
    if (smallBlindPlayer.hasFolded() || bigBlindPlayer.hasFolded()) throw std::logic_error("Cannot resolve showdown after fold");

    if((!smallBlindPlayer.isAllIn() && !bigBlindPlayer.isAllIn()) || 
    state_.actionsToClose() != 0) {
        throw invalid_argument("Hand state is not valid to resolve All-In");
    }

    std::int64_t sbcommit = smallBlindPlayer.committedTotal();
    std::int64_t bbcommit = bigBlindPlayer.committedTotal();

    if(sbcommit > bbcommit) smallBlindPlayer.refundChips(sbcommit - bbcommit);
    else if(sbcommit < bbcommit) bigBlindPlayer.refundChips(bbcommit - sbcommit);

    runoutBoard();

    settleShowdown();
}

void HandEngine::settleFold() {
    if (state_.phase_ != HandPhase::Betting) {
        throw std::logic_error("Hand is not in betting phase");
    }

    const bool folded0 = state_.players_[0].hasFolded();
    const bool folded1 = state_.players_[1].hasFolded();

    if (folded0 == folded1) {
        throw std::logic_error("Exactly one player must have folded");
    }

    if (state_.payouts_[0] != 0 || state_.payouts_[1] != 0) {
        throw std::logic_error("Hand has already been settled");
    }

    const std::uint8_t winner = folded0 ? 1 : 0;
    const std::int64_t pot = state_.pot();

    if (pot <= 0) {
        throw std::logic_error("Cannot settle an empty pot");
    }

    // All committed chips belong to the remaining player.
    state_.players_[winner].awardChips(pot);
    state_.payouts_[winner] = pot;

    state_.phase_ = HandPhase::Finished;
    state_.actingPlayer_ = std::nullopt;
    state_.actionsToClose_ = 0;
}

} // namespace poker