/**
 * @file preflop_equity_table.cc
 * @brief Implementation of preflop hand classification.
 * @author Arian Mohammadian
 */

#include "poker/preflop_equity_table.h"

#include <algorithm>
#include <stdexcept>

namespace poker {

namespace {

#include "preflop_equities_generated.inc"

} // namespace

std::pair<std::size_t, std::size_t>
PreflopEquityTable::index(
    const Card& first,
    const Card& second
) {
    // TODO 1: Reject duplicate cards.
    if(first == second) throw std::invalid_argument("Cards are duplicated");

    // TODO 2: Obtain rank indices in [0, 12].
    int index1 = static_cast<int>(first.rank()) - 2, index2 = static_cast<int>(second.rank()) - 2;

    std::size_t minIndex = std::min(index1, index2);
    std::size_t maxIndex = std::max(index1, index2);

    // TODO 3: Determine whether the hand is suited.
    bool suited = (first.suit() == second.suit());

    // TODO 4: Return the correct matrix coordinates.
    return (suited ? std::pair{minIndex, maxIndex} : std::pair{maxIndex, minIndex});
}

double PreflopEquityTable::lookup(const std::array<Card, 2>& holeCards) {
    const auto [row, col] = index(
        holeCards[0],
        holeCards[1]
    );

    return kEquities[row][col];
}

} // namespace poker