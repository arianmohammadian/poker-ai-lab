/**
 * @file q_table.cc
 * @brief Implements tabular action-value storage and incremental updates.
 * @author Arian Mohammadian
 */

#include "poker/q_table.h"

#include <cmath>
#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>
#include <ios>
#include <cstdint>

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

void QTable::save(const std::string& path) const {
    std::ofstream file(path);

    if (!file)
        throw std::runtime_error("Cannot open QTable file for writing");

    file << "POKER_AI_LAB_QTABLE 1 " << kEntries << '\n';

    file << std::setprecision(17);

    for (std::size_t i = 0; i < kEntries; ++i) {
        file << visits_[i] << ' ' << values_[i] << '\n';
    }

    file.flush();

    if (!file)
        throw std::runtime_error("Failed to write QTable checkpoint");
}

void QTable::load(const std::string& path) {
    std::ifstream file(path);

    if (!file)
        throw std::runtime_error(
            "Cannot open QTable checkpoint for reading"
        );

    std::string magic;
    std::uint32_t version;
    std::size_t entries;

    // Validate checkpoint header.
    if (!(file >> magic >> version >> entries) ||
        magic != "POKER_AI_LAB_QTABLE" ||
        version != 1 ||
        entries != kEntries) {
        throw std::runtime_error(
            "Invalid QTable checkpoint header"
        );
    }

    // Load into temporary arrays to preserve the current
    // table if the checkpoint is invalid.
    std::array<double, kEntries> newValues{};
    std::array<std::uint64_t, kEntries> newVisits{};

    for (std::size_t i = 0; i < kEntries; ++i) {
        if (!(file >> newVisits[i] >> newValues[i])) {
            throw std::runtime_error(
                "Incomplete or invalid QTable checkpoint"
            );
        }

        if (!std::isfinite(newValues[i])) {
            throw std::runtime_error(
                "Non-finite Q-value in checkpoint"
            );
        }

        if (newVisits[i] == 0 && newValues[i] != 0.0) {
            throw std::runtime_error(
                "Unvisited QTable entry has nonzero value"
            );
        }
    }

    // Reject unexpected content after the final entry.
    file >> std::ws;

    if (file.bad()) {
        throw std::runtime_error(
            "Error reading QTable checkpoint"
        );
    }

    if (file.peek() != std::char_traits<char>::eof()) {
        throw std::runtime_error(
            "Unexpected trailing data in QTable checkpoint"
        );
    }

    // Commit changes only after successful validation.
    values_ = newValues;
    visits_ = newVisits;
}

} // namespace poker