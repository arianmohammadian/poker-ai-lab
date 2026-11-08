/**
 * @file preflop_equity_table.h
 * @brief Precomputed preflop equities against a random opponent.
 * @author Arian Mohammadian
 */

#pragma once

#include "poker/card.h"

#include <array>
#include <utility>
#include <cstddef>

namespace poker {

class PreflopEquityTable {
public:
    static constexpr std::size_t kRanks = 13;

    using Table = std::array<
        std::array<double, kRanks>,
        kRanks
    >;

    [[nodiscard]] static std::pair<std::size_t, std::size_t> index(
        const Card& first,
        const Card& second
    );

    [[nodiscard]] static double lookup(
        const std::array<Card, 2>& holeCards
    );
};

} // namespace poker