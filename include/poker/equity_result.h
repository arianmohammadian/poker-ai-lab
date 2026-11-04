/**
 * @file equity_result.h
 * @brief Stores the results of an equity calculation.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>
#include <stdexcept>

namespace poker {

struct EquityResult {
    std::uint64_t player1Wins = 0;
    std::uint64_t player2Wins = 0;
    std::uint64_t ties = 0;

    [[nodiscard]] std::uint64_t total() const {
        return player1Wins + player2Wins + ties;
    }

    [[nodiscard]] double player1Equity() const {
        if(total() == 0) throw std::logic_error("Total can't be zero");
        return (player1Wins + ties / 2.0) / total();
    }

    [[nodiscard]] double player2Equity() const {
        if(total() == 0) throw std::logic_error("Total can't be zero");
        return (player2Wins + ties / 2.0) / total();
    }
};

} // namespace poker