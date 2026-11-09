/**
 * @file q_table.cc
 * @brief Implements tabular action-value storage and incremental updates.
 * @author Arian Mohammadian
 */

#include "poker/q_table.h"

#include <cmath>
#include <stdexcept>

namespace poker {

std::size_t QTable::index(
    const AbstractState& state,
    AbstractAction action
) {
    // TODO 1: Convert Street into an index in [0, 3].
    std::size_t streetIDX = static_cast<std::uint8_t>(state.street);
    if(streetIDX > 3) throw std::invalid_argument("Invalid street");

    // TODO 2: Validate all bucket values.
    if(state.equityBucket > 4 || state.stackBucket > 3 || state.potOddsBucket > 3) {
        throw std::invalid_argument("Abstract State buckets are incorrect");
    }

    // TODO 3: Validate the abstract action.
    std::size_t actionIDX = static_cast<std::size_t>(action);
    if(actionIDX >= kActions) throw std::invalid_argument("Abstract Action is incorrect");

    // TODO 4: Calculate and return the flattened index.
    return 5 * ((((streetIDX * 5 + state.equityBucket) * 4 + state.stackBucket)
    * 4 + state.potOddsBucket) * 2 + state.isButton) + actionIDX;
}

double QTable::value(
    const AbstractState& state,
    AbstractAction action
) const {
    std::size_t idx = index(state, action);
    return values_[idx];
}

std::uint64_t QTable::visits(
    const AbstractState& state,
    AbstractAction action
) const {
    std::size_t idx = index(state, action);
    return visits_[idx];
}

void QTable::update(
    const AbstractState& state,
    AbstractAction action,
    double reward
) {
    // TODO 1: Validate that reward is finite.
    if(!std::isfinite(reward)) throw std::invalid_argument("Reward must be finite");

    // TODO 2: Obtain the flattened index.
    std::size_t idx = index(state, action);

    // TODO 3: Increment the visit counter.
    ++visits_[idx];

    // TODO 4: Update the Q value using the
    // incremental sample mean formula.
    values_[idx] = values_[idx] + (reward - values_[idx]) / visits_[idx];
}

} // namespace poker