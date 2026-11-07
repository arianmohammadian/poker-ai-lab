
/**
 * @file heuristic_agent.h
 * @brief Equity-based heuristic agent for Heads-Up NLHE.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/agent.h"

#include <cstdint>
#include <random>

namespace poker {

enum class HeuristicMode {
    Passive,
    Aggressive
};

struct HeuristicConfig {
    HeuristicMode mode{HeuristicMode::Passive};

    std::uint64_t equitySamples{1000};

    double valueBetThreshold{0.70};
    double valueRaiseThreshold{0.85};

    double bluffFrequency{0.05};
    double betPotFraction{0.50};
    double raisePotFraction{0.75};
};

class HeuristicAgent final : public Agent {
public:
    explicit HeuristicAgent(std::mt19937& rng, HeuristicConfig config = {});

    [[nodiscard]] Action chooseAction(
        const PlayerObservation& observation
    ) override;

private:
    std::mt19937& rng_;
    HeuristicConfig config_;

    [[nodiscard]] Action choosePassiveAction(
        const LegalActions& legal,
        double equity,
        double requiredEquity
    ) const;

    [[nodiscard]] Action chooseAggressiveAction(
        const PlayerObservation& observation,
        double equity,
        double requiredEquity
    );

    [[nodiscard]] std::int64_t calculateBetSize(
        const PlayerObservation& observation
    ) const;

    [[nodiscard]] std::int64_t calculateRaiseSize(
        const PlayerObservation& observation
    ) const;

    [[nodiscard]] bool shouldBluff();
};

} // namespace poker
