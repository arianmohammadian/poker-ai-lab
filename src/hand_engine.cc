#include "poker/hand_engine.h"

#include <stdexcept>
#include <utility>
#include <limits>

namespace poker {

namespace {

HandConfig validateConfigAndInputs(
    HandConfig config,
    const std::array<std::int64_t, 2>& initialStacks,
    std::uint8_t button
) {
    if (config.smallBlind <= 0 || config.bigBlind <= config.smallBlind) {
        throw std::invalid_argument("Blinds must satisfy 0 < SB < BB");
    }

    if (button > 1) {
        throw std::invalid_argument("Button must be 0 or 1");
    }

    if (initialStacks[0] <= 0 || initialStacks[1] <= 0) {
        throw std::invalid_argument("Initial stacks must be positive");
    }

    const auto maxChips = std::numeric_limits<std::int64_t>::max();

    if (initialStacks[0] > maxChips - initialStacks[1]) {
        throw std::invalid_argument("Total chips exceed int64_t capacity");
    }

    return config;
}

HandState createInitialState(
    Deck& deck,
    std::array<std::int64_t, 2> initialStacks,
    std::uint8_t button
) {
    Card c1 = deck.draw();
    Card c2 = deck.draw();
    Card c3 = deck.draw();
    Card c4 = deck.draw();

    std::array<Card, 2> holeCards0 =
        button == 0 ? std::array<Card, 2>{c1, c3}
                    : std::array<Card, 2>{c2, c4};

    std::array<Card, 2> holeCards1 =
        button == 0 ? std::array<Card, 2>{c2, c4}
                    : std::array<Card, 2>{c1, c3};

    return HandState{
        {
            PlayerState{initialStacks[0], holeCards0},
            PlayerState{initialStacks[1], holeCards1}
        },
        button
    };
}

Deck createShuffledDeck(std::mt19937& rng) {
    Deck deck;
    deck.shuffle(rng);

    return deck;
}


} // namespace

HandEngine::HandEngine(
    std::array<std::int64_t, 2> initialStacks,
    HandConfig config,
    std::uint8_t button,
    std::mt19937& rng
)
    : config_(validateConfigAndInputs(config, initialStacks, button)),
      deck_(createShuffledDeck(rng)),
      state_(createInitialState(deck_, initialStacks, button))
{
    
}

const HandState& HandEngine::state() const noexcept {
    return state_;
}

} // namespace poker