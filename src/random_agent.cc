/**
 * @file random_agent.cc
 * @brief Implements random selection of legal poker actions.
 * @author Arian Mohammadian
 */

#include "poker/random_agent.h"

#include <stdexcept>
#include <vector>

namespace poker {

RandomAgent::RandomAgent(std::mt19937& rng)
    : rng_(rng) {}

Action RandomAgent::chooseAction(
    const PlayerObservation& observation
) {
    if (!observation.legalActions.has_value()) {
        throw std::logic_error("Agent cannot act without legal actions");
    }

    const LegalActions& legal = *observation.legalActions;

    std::vector<Action> candidates;

    if(legal.canFold) candidates.push_back({ActionType::Fold, 0});
    if(legal.canCheck) candidates.push_back({ActionType::Fold, 0});
    if(legal.canCall) candidates.push_back({ActionType::Call, 0});
    if(legal.canBet) {
        if(legal.shortAllInBet.has_value()) {
            candidates.push_back({
                ActionType::Bet,
                *legal.shortAllInBet
            });
        } else {
            std::uniform_int_distribution<std::int64_t> dist(
                legal.minBet, legal.maxBet
            );

            candidates.push_back({
                ActionType::Bet,
                dist(rng_)
            });
        }
    }
    if(legal.canRaise) {
        if(legal.shortAllInRaise.has_value()) {
            candidates.push_back({
                ActionType::Raise,
                *legal.shortAllInRaise
            });
        } else {
            std::uniform_int_distribution<std::int64_t> dist(
                legal.minRaise, legal.maxRaise
            );

            candidates.push_back({
                ActionType::Raise,
                dist(rng_)
            });
        }
    }

    if(candidates.empty()) throw std::logic_error("Player does not have any legal actions");

    std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
    return candidates[dist(rng_)];
}

} // namespace poker