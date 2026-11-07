
/**
 * @file heuristic_agent.cc
 * @brief Implementation of the equity-based heuristic agent.
 * @author Arian Mohammadian
 */

#include "poker/heuristic_agent.h"
#include "poker/equity_calculator.h"

#include <stdexcept>

namespace poker {

HeuristicAgent::HeuristicAgent(std::mt19937& rng)
    : rng_(rng) {}

Action HeuristicAgent::chooseAction(
    const PlayerObservation& observation
) {
    if (!observation.legalActions.has_value()) {
        throw std::logic_error("Agent cannot act without legal actions");
    }

    const LegalActions& legal = *observation.legalActions;

    if(legal.canCheck) return {ActionType::Check, 0};

    if(!legal.canCall) return {ActionType::Fold, 0};

    // can call -> decide
    const std::int64_t callCost = std::min(
        legal.toCall,
        observation.stacks[observation.player]
    );

    const std::int64_t unmatched = legal.toCall - callCost;

    const double requiredEquity =
        static_cast<double>(callCost) /
        (observation.pot - unmatched + callCost);

    EquityResult res = EquityCalculator::calculateMonteCarloVsUnknown(
        observation.holeCards,
        observation.board,
        kEquitySamples,
        rng_()
    );
    
    if(res.player1Equity() >= requiredEquity) return {ActionType::Call, 0};
    else return {ActionType::Fold, 0};
}

} // namespace poker
