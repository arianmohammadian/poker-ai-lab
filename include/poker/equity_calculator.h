/**
 * @file equity_calculator.h
 * @brief Calculates Heads-Up Texas Hold'em equity.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/equity_input.h"
#include "poker/equity_result.h"

#include <cstdint>
#include <array>
#include <vector>

namespace poker {

class EquityCalculator {
public:
    [[nodiscard]] static EquityResult calculateExact(
        const EquityInput& input
    );

    [[nodiscard]] static EquityResult calculateMonteCarlo(
        const EquityInput& input,
        std::uint64_t samples,
        std::uint64_t seed
    );

    [[nodiscard]] static EquityResult calculateMonteCarloVsUnknown(
        const std::array<Card, 2>& hero,
        const std::vector<Card>& board,
        std::uint64_t samples,
        std::uint64_t seed
    );
};

} // namespace poker