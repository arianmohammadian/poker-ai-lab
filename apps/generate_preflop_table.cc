
/**
 * @file generate_preflop_table.cc
 * @brief Generates precomputed preflop equities using Monte Carlo.
 * @author Arian Mohammadian
 */

#include "poker/preflop_equity_table.h"
#include "poker/equity_calculator.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kRanks = 13;
constexpr std::uint64_t kBaseSeed = 42;

constexpr char kRankSymbols[] = "23456789TJQKA";

std::array<poker::Card, 2> representativeHand(
    std::size_t row,
    std::size_t col
) {
    const auto rank1 = static_cast<poker::Rank>(row + 2);
    const auto rank2 = static_cast<poker::Rank>(col + 2);

    // Pairs: two different suits.
    if (row == col) {
        return {
            poker::Card(rank1, poker::Suit::Clubs),
            poker::Card(rank2, poker::Suit::Diamonds)
        };
    }

    // Upper triangle: suited hands.
    if (row < col) {
        return {
            poker::Card(rank1, poker::Suit::Clubs),
            poker::Card(rank2, poker::Suit::Clubs)
        };
    }

    // Lower triangle: offsuit hands.
    return {
        poker::Card(rank1, poker::Suit::Clubs),
        poker::Card(rank2, poker::Suit::Diamonds)
    };
}

std::string handName(std::size_t row, std::size_t col) {
    const std::size_t high = std::max(row, col);
    const std::size_t low = std::min(row, col);

    std::string name;
    name += kRankSymbols[high];
    name += kRankSymbols[low];

    if (row != col) {
        name += (row < col ? 's' : 'o');
    }

    return name;
}

} // namespace

int main() {
    try {
        std::uint64_t samples = 0;

        std::cout
            << "\n========================================\n"
            << " POKER AI LAB - PREFLOP EQUITY GENERATOR\n"
            << "========================================\n\n"
            << "Monte Carlo samples per hand class: ";

        if (!(std::cin >> samples) || samples == 0) {
            throw std::invalid_argument(
                "Samples must be a positive integer"
            );
        }

        const std::filesystem::path outputPath =
            "data/preflop_equities.csv";

        std::filesystem::create_directories(
            outputPath.parent_path()
        );

        std::ofstream output(outputPath);

        if (!output) {
            throw std::runtime_error(
                "Cannot open output file"
            );
        }

        output << std::setprecision(17);
        output << "row,col,hand,equity,samples,seed\n";

        std::size_t completed = 0;

        for (std::size_t row = 0; row < kRanks; ++row) {
            for (std::size_t col = 0; col < kRanks; ++col) {

                const auto holeCards =
                    representativeHand(row, col);

                // Verify consistency with our classification.
                const auto [actualRow, actualCol] =
                    poker::PreflopEquityTable::index(
                        holeCards[0], holeCards[1]
                    );

                if (actualRow != row || actualCol != col) {
                    throw std::logic_error(
                        "Preflop hand classification mismatch"
                    );
                }

                // Separate deterministic seed for each class.
                const std::uint64_t seed =
                    kBaseSeed + row * kRanks + col;

                const auto result =
                    poker::EquityCalculator::
                        calculateMonteCarloVsUnknown(
                            holeCards,
                            std::vector<poker::Card>{},
                            samples,
                            seed
                        );

                const double equity = result.player1Equity();
                const std::string name = handName(row, col);

                output
                    << row << ','
                    << col << ','
                    << name << ','
                    << equity << ','
                    << samples << ','
                    << seed << '\n';

                ++completed;

                std::cout
                    << '[' << completed << "/169] "
                    << std::setw(3) << name
                    << "  Equity: "
                    << std::fixed << std::setprecision(3)
                    << equity * 100.0 << "%\n";
            }
        }

        output.close();

        if (!output) {
            throw std::runtime_error(
                "Failed to write preflop equity table"
            );
        }

        std::cout
            << "\n========================================\n"
            << "Generated 169 preflop equities.\n"
            << "Saved to: " << outputPath.string() << '\n'
            << "Samples per class: " << samples << '\n'
            << "Total simulations: "
            << (169.0L * static_cast<long double>(samples))
            << "\n========================================\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr
            << "Preflop generator error: "
            << e.what() << '\n';

        return 1;
    }
}
