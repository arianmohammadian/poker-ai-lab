#include "poker/abstract_state.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace poker {

std::uint8_t StateAbstractor::equityBucket(double equity) {
    if(!std::isfinite(equity) || equity < 0 || equity > 1) throw std::invalid_argument("Equity must belong to [0, 1]");
    if(equity < 0.2) return 0;
    else if(equity < 0.4) return 1;
    else if(equity < 0.6) return 2;
    else if(equity < 0.8) return 3;
    else return 4;
}

std::uint8_t StateAbstractor::stackBucket(
    const PlayerObservation& observation,
    std::int64_t bigBlind
) {
    if(bigBlind <= 0) throw std::invalid_argument("Big blind must be > 0");
    std::int64_t lowerStack = std::min(observation.stacks[0], observation.stacks[1]);
    double stackDef = static_cast<double>(lowerStack) / bigBlind;
    if(stackDef < 10) return 0;
    else if(stackDef < 25) return 1;
    else if(stackDef < 75) return 2;
    else return 3;
}

std::uint8_t StateAbstractor::potOddsBucket(
    const PlayerObservation& observation
) {
    if (!observation.legalActions) throw std::invalid_argument("Legal actions are required");

    const auto& legal = *observation.legalActions;

    if(legal.toCall == 0) return 0;

    double potOdds = static_cast<double>(legal.toCall) / (observation.pot + legal.toCall);

    if(potOdds < 0.1) return 0;
    else if (potOdds < 0.25) return 1;
    else if(potOdds < 0.5) return 2;
    else return 3;
}

AbstractState StateAbstractor::abstract(
    const PlayerObservation& observation,
    double equity,
    std::int64_t bigBlind
) {
    return {
        observation.street,
        equityBucket(equity),
        stackBucket(observation, bigBlind),
        potOddsBucket(observation),
        observation.player == observation.button
    };
}

} // namespace poker