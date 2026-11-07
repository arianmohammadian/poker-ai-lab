
/**
 * @file heuristic_agent.cc
 * @brief Implementation of the equity-based heuristic agent.
 * @author Arian Mohammadian
 */

#include "poker/heuristic_agent.h"
#include "poker/equity_calculator.h"

#include <stdexcept>

namespace poker {

namespace {

void validateConfigParams(const HeuristicConfig& config) {
    auto validProbability = [](double value) {
        return std::isfinite(value) &&
               value >= 0.0 &&
               value <= 1.0;
    };

    auto validFraction = [](double value) {
        return std::isfinite(value) && value > 0.0;
    };

    if (config.equitySamples == 0 ||
        !validProbability(config.valueBetThreshold) ||
        !validProbability(config.valueRaiseThreshold) ||
        !validProbability(config.bluffFrequency) ||
        !validFraction(config.betPotFraction) ||
        !validFraction(config.raisePotFraction)) {
        throw std::invalid_argument("Invalid heuristic configuration");
    }
}

}

HeuristicAgent::HeuristicAgent(
    std::mt19937& rng,
    HeuristicConfig config
)
    : rng_(rng),
      config_(config) {

    validateConfigParams(config);
}

HeuristicAgent::HeuristicAgent(
    std::mt19937& rng,
    HeuristicMode mode
)
    : HeuristicAgent(rng, HeuristicConfig{.mode = mode}) {}

std::int64_t HeuristicAgent::calculateBetSize(
    const PlayerObservation& observation
) const {
    const LegalActions& legal = *observation.legalActions;

    const double target =
        config_.betPotFraction *
        static_cast<double>(observation.pot);

    if (target <= static_cast<double>(legal.minBet))
        return legal.minBet;

    if (target >= static_cast<double>(legal.maxBet))
        return legal.maxBet;

    return static_cast<std::int64_t>(target);
}

std::int64_t HeuristicAgent::calculateRaiseSize(
    const PlayerObservation& observation
) const {
    const LegalActions& legal = *observation.legalActions;

    std::uint8_t opponent = 1 - observation.player;
    std::int64_t oppCommitment = observation.committedStreet[opponent];

    const long double target =
        static_cast<long double>(oppCommitment) +
        static_cast<long double>(config_.raisePotFraction) *
        static_cast<long double>(observation.pot);

    if (target <= static_cast<long double>(legal.minRaise))
        return legal.minRaise;

    if (target >= static_cast<long double>(legal.maxRaise))
        return legal.maxRaise;

    return static_cast<std::int64_t>(target);
}

bool HeuristicAgent::shouldBluff() {
    std::bernoulli_distribution dist(config_.bluffFrequency);
    return dist(rng_);
}

Action HeuristicAgent::choosePassiveAction(const LegalActions& legal,
double equity, double requiredEquity) const {
    if (legal.canCheck)
        return {ActionType::Check, 0};

    if (legal.canCall && equity >= requiredEquity)
        return {ActionType::Call, 0};

    if (legal.canFold)
        return {ActionType::Fold, 0};

    throw std::logic_error("No legal passive action available");
}


Action HeuristicAgent::chooseAggressiveAction(const PlayerObservation& observation,
double equity, double requiredEquity) {
    const LegalActions& legal = *observation.legalActions;

    // 1. Consider raising with very strong equity.
    if (legal.canRaise &&
        (equity >= config_.valueRaiseThreshold || shouldBluff())) {

        if (legal.shortAllInRaise.has_value()) {
            return {
                ActionType::Raise,
                *legal.shortAllInRaise
            };
        }

        return {
            ActionType::Raise,
            calculateRaiseSize(observation)
        };
    }

    // 2. Consider betting when the pot is unopened.
    if (legal.canBet &&
        (equity >= config_.valueBetThreshold || shouldBluff())) {

        if (legal.shortAllInBet.has_value()) {
            return {
                ActionType::Bet,
                *legal.shortAllInBet
            };
        }

        return {
            ActionType::Bet,
            calculateBetSize(observation)
        };
    }

    // 3. Continue for free if possible.
    if (legal.canCheck)
        return {ActionType::Check, 0};

    // 4. Otherwise, decide between Call and Fold.
    if(legal.canCall && equity >= requiredEquity) return {ActionType::Call, 0};
    else return {ActionType::Fold, 0};
}


Action HeuristicAgent::chooseAction(const PlayerObservation& observation) {
    if (!observation.legalActions) {
        throw std::logic_error(
            "Agent cannot act without legal actions"
        );
    }

    const LegalActions& legal = *observation.legalActions;

    // Avoid Monte Carlo when a passive decision is immediate.
    if (config_.mode == HeuristicMode::Passive) {
        if (legal.canCheck)
            return {ActionType::Check, 0};

        if (!legal.canCall)
            return choosePassiveAction(legal, 0.0, 1.0);
    }

    // If no action other than Fold is available,
    // avoid an unnecessary equity calculation.
    if (!legal.canCheck && !legal.canCall &&
        !legal.canBet && !legal.canRaise) {
        if (legal.canFold)
            return {ActionType::Fold, 0};

        throw std::logic_error("No legal action available");
    }

    double requiredEquity = 0.0;

    if (legal.canCall) {
        const std::int64_t callCost = std::min(
            legal.toCall,
            observation.stacks[observation.player]
        );

        const std::int64_t unmatched = legal.toCall - callCost;
        const std::int64_t contestablePot =
            observation.pot - unmatched;

        if (callCost <= 0 || contestablePot < 0) {
            throw std::logic_error("Invalid call pot odds");
        }

        requiredEquity =
            static_cast<double>(callCost) /
            (static_cast<double>(contestablePot) + callCost);
    }

    const EquityResult result =
        EquityCalculator::calculateMonteCarloVsUnknown(
            observation.holeCards,
            observation.board,
            config_.equitySamples,
            rng_()
        );

    const double equity = result.player1Equity();

    if (config_.mode == HeuristicMode::Passive) return choosePassiveAction(legal, equity, requiredEquity);
    else return chooseAggressiveAction(observation, equity, requiredEquity);
}


} // namespace poker
