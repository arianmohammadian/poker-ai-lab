/**
 * @file equity_calculator.h
 * @brief Calculates Heads-Up Texas Hold'em equity.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/equity_input.h"
#include "poker/equity_result.h"

#include <cstdint>

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
};

} // namespace poker