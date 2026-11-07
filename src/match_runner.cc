/**
 * @file match_runner.cc
 * @brief Executes reproducible matches between poker agents.
 * @author Arian Mohammadian
 */

#include "poker/match_runner.h"
#include "poker/hand_engine.h"
#include "poker/game_engine.h"
#include "poker/hand_phase.h"

#include <array>
#include <random>
#include <stdexcept>

namespace poker {

MatchResult MatchRunner::run(
    Agent& agentA,
    Agent& agentB,
    const MatchConfig& config
) {
    MatchResult result{};

    std::mt19937 deckRng(config.seed);

    for (std::uint64_t gameIndex = 0; gameIndex < config.games; ++gameIndex) {

        // TODO 1: Create a new GameEngine.
        GameEngine game(
            {config.startingStack, config.startingStack},
            {config.smallBlind, config.bigBlind},
            0,
            deckRng
        );

        // TODO 2: Assign agents to seats.
        const std::uint8_t seatA = gameIndex % 2;

        std::uint64_t actions = 0;
        constexpr std::uint64_t kMaxActions = 100000;

        // TODO 3: Play until the game finishes.
        while(!game.isFinished()) {
            if (++actions > kMaxActions) throw std::runtime_error("Match exceeded action limit");

            const auto& state = game.currentHand().state();
            if (state.phase() == HandPhase::Finished) {
                game.nextHand();
                continue;
            }

            const auto actor = state.actingPlayer();

            if (!actor.has_value())
                throw std::logic_error("No acting player during betting");

            const std::uint8_t player = *actor;

            // TODO 1: Obtain the player's observation.
            const PlayerObservation observation = game.currentHand().observe(player);

            // TODO 2: Determine whether player == seatA or seatB.
            Agent& currentAgent = (player == seatA) ? agentA : agentB;

            // TODO 3: Ask the corresponding agent for an Action.
            const Action action = currentAgent.chooseAction(observation);

            // TODO 4: Execute the action using game.applyAction().
            game.applyAction(player, action);
        }

        // TODO 4: Record the winner and statistics.
        const auto& state = game.currentHand().state();

        const std::int64_t stackA = state.player(seatA).stack();
        const std::int64_t stackB = state.player(1 - seatA).stack();

        if (stackA > stackB)
            ++result.winsA;
        else if (stackB > stackA)
            ++result.winsB;
        else
            ++result.ties;

        ++result.gamesPlayed;
        result.totalHands += game.handNumber();
    }

    return result;
}

} // namespace poker