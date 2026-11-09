/**
 * @file abstract_state.h
 * @brief Defines a compact state representation for tabular poker agents.
 * @author Arian Mohammadian
 */

#pragma once

#include "poker/player_observation.h"

#include <cstdint>

namespace poker {

struct AbstractState {
    Street street;
    std::uint8_t equityBucket;
    std::uint8_t stackBucket;
    std::uint8_t potOddsBucket;
    bool isButton;

    bool operator==(const AbstractState&) const = default;
};

class StateAbstractor {
public:
    [[nodiscard]] static AbstractState abstract(
        const PlayerObservation& observation,
        double equity,
        std::int64_t bigBlind
    );

private:
    [[nodiscard]] static std::uint8_t equityBucket(double equity);

    [[nodiscard]] static std::uint8_t stackBucket(
        const PlayerObservation& observation,
        std::int64_t bigBlind
    );

    [[nodiscard]] static std::uint8_t potOddsBucket(
        const PlayerObservation& observation
    );
};

} // namespace poker