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

PlayerObservation HandEngine::observe(std::uint8_t player) const {
    if (player >= 2) {
        throw std::invalid_argument("Invalid player index");
    }

    std::optional<LegalActions> legal;

    if (state_.phase() == HandPhase::Betting &&
        state_.actingPlayer() == player) {
        legal = legalActions(player);
    }

    return PlayerObservation{
        player,
        state_.player(player).holeCards(),
        state_.board(),
        {state_.players_[0].stack(), state_.players_[1].stack()},
        {state_.players_[0].committedStreet(), state_.players_[1].committedStreet()},
        state_.pot(),
        state_.street(),
        state_.button(),
        legal
    };
}

LegalActions HandEngine::legalActions(std::uint8_t player) const {
    if (player >= 2) {
        throw std::invalid_argument("Invalid player index");
    }

    if (state_.phase() != HandPhase::Betting ||
        state_.actingPlayer() != player) {
        throw std::invalid_argument("It is not this player's turn");
    }

    const PlayerState& current = state_.player(player);
    const PlayerState& opponent = state_.player(1 - player);

    const std::int64_t mine = current.committedStreet();
    const std::int64_t theirs = opponent.committedStreet();

    LegalActions legal;

    legal.toCall = std::max<std::int64_t>(0, theirs - mine);

    legal.canFold = true;

    legal.canCheck = (legal.toCall == 0);

    legal.canCall = (legal.toCall > 0 && current.stack() > 0);

    legal.canBet = (mine == 0 && theirs == 0 &&
    current.stack() > 0 && !opponent.isAllIn());

    if (legal.canBet) {
        const std::int64_t minTotal = config_.bigBlind;
        const std::int64_t maxTotal = mine + current.stack();

        if (maxTotal >= minTotal) {
            legal.minBet = minTotal;
            legal.maxBet = maxTotal;
        } else {
            legal.shortAllInBet = maxTotal;
        }
    }

    legal.canRaise = (mine > 0 || theirs > 0) && !opponent.isAllIn() &&
    mine + current.stack() > theirs;

    if (legal.canRaise) {
        const std::int64_t maxTotal = mine + current.stack();
        const std::int64_t minTotal = theirs + state_.lastFullRaise();

        if (maxTotal >= minTotal) {
            legal.minRaise = minTotal;
            legal.maxRaise = maxTotal;
        } else {
            legal.shortAllInRaise = maxTotal;
        }
    }

    return legal;
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
    if (state_.phase_ != HandPhase::Betting)
        throw std::logic_error("Hand is not accepting actions");
    if (!state_.actingPlayer_.has_value() || *state_.actingPlayer_ != player)
        throw std::invalid_argument("It is not this player's turn");
    if (state_.actionsToClose_ == 0)
        throw std::logic_error("No betting actions pending");

    PlayerState& current = state_.players_[player];
    PlayerState& opponent = state_.players_[1 - player];
    if (current.hasFolded() || current.isAllIn())
        throw std::logic_error("Player cannot act");

    const std::int64_t mine = current.committedStreet();
    const std::int64_t theirs = opponent.committedStreet();
    if (mine > theirs)
        throw std::logic_error("Unmatched excess commitment while acting");
    const std::int64_t toCall = theirs - mine;

    if (action.type == ActionType::Fold ||
        action.type == ActionType::Check ||
        action.type == ActionType::Call) {
        if (action.amount != 0)
            throw std::invalid_argument("Fold, Check and Call require amount 0");
    }

    // Only called AFTER validating and committing a legal non-aggressive action.
    auto completePassiveAction = [&]() {
        --state_.actionsToClose_;
        if (state_.actionsToClose_ == 0 ||
            current.isAllIn() || opponent.isAllIn()) {
            state_.actionsToClose_ = 0;
            state_.actingPlayer_ = std::nullopt;
            advanceStreet();
        } else {
            state_.actingPlayer_ = static_cast<std::uint8_t>(1 - player);
        }
    };

    switch (action.type) {
        case ActionType::Fold:
            current.fold();
            settleFold();
            return;

        case ActionType::Check:
            if (toCall != 0)
                throw std::invalid_argument("Cannot check facing a bet");
            completePassiveAction();
            return;

        case ActionType::Call: {
            if (toCall == 0)
                throw std::invalid_argument("Nothing to call; use Check");
            const std::int64_t paid = std::min(toCall, current.stack());
            // current.stack() > 0 because current.isAllIn() was rejected above.
            current.commitChips(paid);
            completePassiveAction();
            return;
        }

        case ActionType::Bet: {
            if (mine != 0 || theirs != 0)
                throw std::invalid_argument("Bet requires an unopened street");
            if (opponent.isAllIn())
                throw std::invalid_argument("Cannot bet against an all-in opponent");
            if (action.amount <= 0 || action.amount > current.stack())
                throw std::invalid_argument("Bet exceeds stack or is nonpositive");
            // Below-BB opening bets are legal only when truly all-in.
            if (action.amount < config_.bigBlind && action.amount != current.stack())
                throw std::invalid_argument("Bet below the minimum without all-in");

            current.commitChips(action.amount);
            // An all-in short opening bet is NOT a full raise increment.
            if (action.amount >= config_.bigBlind)
                state_.lastFullRaise_ = action.amount;
            state_.actionsToClose_ = 1;
            state_.actingPlayer_ = static_cast<std::uint8_t>(1 - player);
            return;
        }

        case ActionType::Raise: {
            if (mine == 0 && theirs == 0)
                throw std::invalid_argument("Use Bet to open betting");
            if (opponent.isAllIn())
                throw std::invalid_argument("Cannot raise an all-in opponent");
            // Raise.amount is the player's TARGET street commitment.
            // Calculate with subtraction to avoid overflow of mine + stack.
            if (action.amount <= theirs || action.amount <= mine)
                throw std::invalid_argument("Raise must exceed the current bet");
            const std::int64_t additional = action.amount - mine;
            if (additional > current.stack())
                throw std::invalid_argument("Raise exceeds player's stack");
            const std::int64_t increment = action.amount - theirs;
            const bool allIn = additional == current.stack();
            const bool fullRaise = increment >= state_.lastFullRaise_;
            if (!fullRaise && !allIn)
                throw std::invalid_argument("Raise below minimum without all-in");

            current.commitChips(additional);
            if (fullRaise)
                state_.lastFullRaise_ = increment;
            // In HU, if the raiser is all-in, the other player can only
            // fold/call: it is impossible to re-raise against a single all-in.
            state_.actionsToClose_ = 1;
            state_.actingPlayer_ = static_cast<std::uint8_t>(1 - player);
            return;
        }
    }

    throw std::invalid_argument("Unknown action type");
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
    while (state_.board_.size() < 5) {
        switch (state_.board_.size()) {
            case 0:
                static_cast<void>(deck_.draw()); // Burn before flop
                for (int i = 0; i < 3; ++i)
                    state_.board_.push_back(deck_.draw());
                state_.street_ = Street::Flop;
                break;
            case 3:
                static_cast<void>(deck_.draw()); // Burn before turn
                state_.board_.push_back(deck_.draw());
                state_.street_ = Street::Turn;
                break;
            case 4:
                static_cast<void>(deck_.draw()); // Burn before river
                state_.board_.push_back(deck_.draw());
                state_.street_ = Street::River;
                break;
            default:
                throw std::logic_error("Invalid community-card count");
        }
    }
    if (state_.board_.size() != 5)
        throw std::logic_error("Invalid community-card count");
    state_.street_ = Street::River;
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

void HandEngine::advanceStreet() {
    if (state_.phase_ != HandPhase::Betting || state_.actionsToClose_ != 0 ||
        state_.actingPlayer_.has_value())
        throw std::logic_error("Cannot advance while betting is open");

    PlayerState& p0 = state_.players_[0];
    PlayerState& p1 = state_.players_[1];
    if (p0.hasFolded() || p1.hasFolded())
        throw std::logic_error("Cannot advance after fold");

    if (p0.isAllIn() || p1.isAllIn()) {
        resolveAllIn();
        return;
    }

    if (p0.committedStreet() != p1.committedStreet())
        throw std::logic_error("Unmatched bets at closed round");

    if (state_.street_ == Street::River) {
        settleShowdown();
        return;
    }

    dealNextStreet();
    p0.beginNewStreet();
    p1.beginNewStreet();
    state_.actingPlayer_ = static_cast<std::uint8_t>(1 - state_.button_);
    state_.actionsToClose_ = 2;
    state_.lastFullRaise_ = config_.bigBlind;
}

void HandEngine::dealNextStreet() {
    if (state_.phase_ != HandPhase::Betting)
        throw std::logic_error("Cannot deal outside betting phase");

    switch (state_.street_) {
        case Street::Preflop:
            static_cast<void>(deck_.draw()); // Burn
            for (int i = 0; i < 3; ++i)
                state_.board_.push_back(deck_.draw());
            state_.street_ = Street::Flop;
            break;
        case Street::Flop:
            static_cast<void>(deck_.draw()); // Burn
            state_.board_.push_back(deck_.draw());
            state_.street_ = Street::Turn;
            break;
        case Street::Turn:
            static_cast<void>(deck_.draw()); // Burn
            state_.board_.push_back(deck_.draw());
            state_.street_ = Street::River;
            break;
        case Street::River:
            throw std::logic_error("Cannot deal beyond river");
    }
}

} // namespace poker