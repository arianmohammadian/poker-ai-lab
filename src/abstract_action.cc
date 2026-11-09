/**
 * @file abstract_action.cc
 * @brief Implements the conversion from abstract to concrete poker actions.
 * @author Arian Mohammadian
 */

#include "poker/abstract_action.h"

#include <stdexcept>
#include <algorithm>

namespace poker {

namespace {

std::int64_t clampBetSize(
    std::int64_t base,
    std::int64_t extra,
    std::int64_t minimum,
    std::int64_t maximum
) {
    // Avoid exceeding the maximum before adding.
    const std::int64_t available = maximum - base;

    const std::int64_t target =
        base + std::min(extra, available);

    return std::clamp(target, minimum, maximum);
}

} // namespace

std::vector<AbstractAction> ActionAbstractor::legalActions(
    const PlayerObservation& observation
) {
    if (!observation.legalActions)
        throw std::logic_error("Missing legal actions");

    constexpr std::array<AbstractAction, 5> candidates = {
        AbstractAction::Fold,
        AbstractAction::CheckCall,
        AbstractAction::HalfPot,
        AbstractAction::Pot,
        AbstractAction::AllIn
    };

    std::vector<AbstractAction> result;
    std::vector<Action> concreteActions;

    for (AbstractAction candidate : candidates) {
        try {
            const Action concrete = toConcrete(candidate, observation);

            // Check whether another abstract action produces
            // exactly the same concrete action.
            bool duplicate = false;

            for (const Action& previous : concreteActions) {
                if (previous.type == concrete.type &&
                    previous.amount == concrete.amount) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                result.push_back(candidate);
                concreteActions.push_back(concrete);
            }

        } catch (const std::invalid_argument&) {
            // This abstract action is not available.
        }
    }

    return result;
}

Action ActionAbstractor::toConcrete(
    AbstractAction action,
    const PlayerObservation& observation
) {
    if (!observation.legalActions)
        throw std::logic_error("Missing legal actions");

    const LegalActions& legal = *observation.legalActions;

    switch (action) {
        case AbstractAction::CheckCall:
            if (legal.canCheck)
                return {ActionType::Check, 0};

            if (legal.canCall)
                return {ActionType::Call, 0};

            throw std::invalid_argument("CheckCall is not legal");

        case AbstractAction::Fold:
            if (legal.canFold) return {ActionType::Fold, 0};

            throw std::invalid_argument("Fold is not legal");

        case AbstractAction::HalfPot:
        case AbstractAction::Pot: {
            const bool halfPot = (action == AbstractAction::HalfPot);

            const std::int64_t potAfterCall =
                observation.pot + legal.toCall;

            const std::int64_t betSize =
                halfPot ? potAfterCall / 2 : potAfterCall;

            // Unopened betting round.
            if (legal.canBet) {
                if (legal.shortAllInBet.has_value())
                    return {ActionType::Bet, *legal.shortAllInBet};

                const std::int64_t amount = clampBetSize(
                    0,
                    betSize,
                    legal.minBet,
                    legal.maxBet
                );

                return {ActionType::Bet, amount};
            }

            // Facing an existing commitment.
            if (legal.canRaise) {
                if (legal.shortAllInRaise.has_value())
                    return {ActionType::Raise, *legal.shortAllInRaise};

                const std::int64_t opponentCommitment =
                    observation.committedStreet[1 - observation.player];

                const std::int64_t amount = clampBetSize(
                    opponentCommitment,
                    betSize,
                    legal.minRaise,
                    legal.maxRaise
                );

                return {ActionType::Raise, amount};
            }

            throw std::invalid_argument(
                "Pot-sized bet or raise is not legal"
            );
        }

        case AbstractAction::AllIn: {
            const std::uint8_t player = observation.player;

            const std::int64_t allInTotal = observation.committedStreet[player] + observation.stacks[player];

            if(legal.canBet) return {ActionType::Bet, allInTotal};

            if(legal.canRaise) return {ActionType::Raise, allInTotal};

            if(legal.canCall && legal.toCall >= observation.stacks[player]) return {ActionType::Call, 0};

            throw std::invalid_argument("All-in is not legal");
        }
    }

    throw std::invalid_argument("Unknown abstract action");
}

} // namespace poker