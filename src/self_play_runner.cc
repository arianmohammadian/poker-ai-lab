#include "poker/self_play_runner.h"

#include "poker/hand_engine.h"
#include "poker/hand_phase.h"

#include <random>
#include <stdexcept>

namespace poker {

EpisodeResult SelfPlayRunner::runHand(
    Agent& agent0,
    Agent& agent1,
    std::array<std::int64_t, 2> stacks,
    HandConfig config,
    std::uint8_t button,
    std::uint32_t seed
) {
    std::mt19937 rng(seed);

    HandEngine engine(stacks, config, button, rng);

    EpisodeResult result{};
    result.initialStacks = stacks;

    // TODO 1: Ejecutar acciones mientras la mano no termine.
    const auto& state = engine.state();
    while(state.phase() != HandPhase::Finished) {
        const auto actor = state.actingPlayer();

        if (!actor.has_value())
            throw std::logic_error("No acting player during betting");

        const std::uint8_t player = *actor;

        const PlayerObservation observation = engine.observe(player);

        Agent& agent = (player == 0) ? agent0 : agent1;

        const Action action = agent.chooseAction(observation);

        engine.applyAction(player, action);

        ++result.decisions;
    }

    result.finalStacks[0] = state.player(0).stack();
    result.finalStacks[1] = state.player(1).stack();

    result.rewards[0] = result.finalStacks[0] - result.initialStacks[0];

    result.rewards[1] = result.finalStacks[1] - result.initialStacks[1];

    return result;
}

} // namespace poker