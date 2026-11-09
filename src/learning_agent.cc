
/**
 * @file learning_agent.cc
 * @brief Implements an epsilon-greedy tabular learning agent.
 * @author Arian Mohammadian
 */

#include "poker/learning_agent.h"

#include "poker/abstract_action.h"
#include "poker/abstract_state.h"
#include "poker/equity_calculator.h"
#include "poker/preflop_equity_table.h"

#include <cmath>
#include <stdexcept>

namespace poker {

LearningAgent::LearningAgent(
    std::mt19937& rng,
    QTable& table,
    LearningConfig config
)
    : rng_(rng),
      table_(table),
      config_(config) {

    if (!std::isfinite(config_.epsilon) ||
        config_.epsilon < 0.0 ||
        config_.epsilon > 1.0 ||
        config_.equitySamples == 0 ||
        config_.bigBlind <= 0) {
        throw std::invalid_argument(
            "Invalid learning configuration"
        );
    }
}

AbstractAction LearningAgent::selectAction(
    const AbstractState& state,
    const std::vector<AbstractAction>& legal
) {
    if (legal.empty())
        throw std::logic_error("No legal abstract actions");

    // Explore with probability epsilon.
    std::bernoulli_distribution explore(config_.epsilon);

    if (explore(rng_)) {
        std::uniform_int_distribution<std::size_t> dist(
            0, legal.size() - 1
        );

        return legal[dist(rng_)];
    }

    // Exploit: find the maximum Q value.
    double bestValue = table_.value(state, legal[0]);

    std::vector<AbstractAction> bestActions{legal[0]};

    for (std::size_t i = 1; i < legal.size(); ++i) {
        const double q = table_.value(state, legal[i]);

        if (q > bestValue) {
            bestValue = q;
            bestActions.clear();
            bestActions.push_back(legal[i]);
        } else if (q == bestValue) {
            bestActions.push_back(legal[i]);
        }
    }

    // Break ties uniformly at random.
    std::uniform_int_distribution<std::size_t> dist(
        0, bestActions.size() - 1
    );

    return bestActions[dist(rng_)];
}

Action LearningAgent::chooseAction(
    const PlayerObservation& observation
) {
    // 1. Estimate equity.
    double equity;

    if (observation.board.empty()) {
        equity = PreflopEquityTable::lookup(
            observation.holeCards
        );
    } else {
        const EquityResult result =
            EquityCalculator::calculateMonteCarloVsUnknown(
                observation.holeCards,
                observation.board,
                config_.equitySamples,
                rng_()
            );

        equity = result.player1Equity();
    }

    // 2. Abstract the observation.
    const AbstractState state = StateAbstractor::abstract(
        observation,
        equity,
        config_.bigBlind
    );

    // 3. Obtain legal abstract actions.
    const auto legal =
        ActionAbstractor::legalActions(observation);

    // 4. Select an action using epsilon-greedy.
    const AbstractAction selected = selectAction(
        state,
        legal
    );

    // 5. Convert it into a concrete poker action.
    const Action concrete =
        ActionAbstractor::toConcrete(selected, observation);

    // 6. Remember the decision for learning.
    experiences_.push_back({state, selected});

    return concrete;
}

void LearningAgent::finishEpisode(double reward) {
    if (!std::isfinite(reward))
        throw std::invalid_argument("Reward must be finite");

    for (const Experience& experience : experiences_) {
        table_.update(
            experience.state,
            experience.action,
            reward
        );
    }

    experiences_.clear();
}

void LearningAgent::clearEpisode() {
    experiences_.clear();
}

} // namespace poker
