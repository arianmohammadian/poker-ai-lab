
/**
 * @file poker_hand_viewer.cc
 * @brief Displays an autonomous Heads-Up NLHE hand in the terminal.
 * @author Arian Mohammadian
 */

#include "poker/hand_engine.h"
#include "poker/random_agent.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>

using poker::Action;
using poker::ActionType;
using poker::Card;
using poker::HandConfig;
using poker::HandEngine;
using poker::HandPhase;
using poker::HandState;
using poker::RandomAgent;
using poker::Rank;
using poker::Street;
using poker::Suit;

namespace {

// --------------------------------------------------
// Card formatting
// --------------------------------------------------

char rankToChar(Rank rank) {
    switch (rank) {
        case Rank::Two:   return '2';
        case Rank::Three: return '3';
        case Rank::Four:  return '4';
        case Rank::Five:  return '5';
        case Rank::Six:   return '6';
        case Rank::Seven: return '7';
        case Rank::Eight: return '8';
        case Rank::Nine:  return '9';
        case Rank::Ten:   return 'T';
        case Rank::Jack:  return 'J';
        case Rank::Queen: return 'Q';
        case Rank::King:  return 'K';
        case Rank::Ace:   return 'A';
    }

    throw std::invalid_argument("Invalid card rank");
}

char suitToChar(Suit suit) {
    switch (suit) {
        case Suit::Clubs:    return 'c';
        case Suit::Diamonds: return 'd';
        case Suit::Hearts:   return 'h';
        case Suit::Spades:   return 's';
    }

    throw std::invalid_argument("Invalid card suit");
}

void printCard(const Card& card) {
    std::cout << rankToChar(card.rank())
              << suitToChar(card.suit());
}

void printBoard(const HandState& state) {
    for (const Card& card : state.board()) {
        printCard(card);
        std::cout << ' ';
    }
}

// --------------------------------------------------
// Hand presentation
// --------------------------------------------------

const char* streetName(Street street) {
    switch (street) {
        case Street::Preflop: return "PREFLOP";
        case Street::Flop:    return "FLOP";
        case Street::Turn:    return "TURN";
        case Street::River:   return "RIVER";
    }

    throw std::invalid_argument("Invalid street");
}

void printInitialState(
    const HandState& state,
    const HandConfig& config
) {
    std::cout << "Blinds: "
              << config.smallBlind << '/'
              << config.bigBlind << "\n";

    for (int i = 0; i < 2; ++i) {
        std::cout << "Player " << i << ": ";

        const auto& cards = state.player(i).holeCards();

        printCard(cards[0]);
        std::cout << ' ';
        printCard(cards[1]);

        std::cout << "    Stack: "
                  << state.player(i).stack()
                  << '\n';
    }

    std::cout << '\n';
}

void printStreet(const HandState& state) {
    std::cout << "\n[" << streetName(state.street()) << "]";

    if (!state.board().empty()) {
        std::cout << ' ';
        printBoard(state);
    }

    std::cout << "\nPot: " << state.pot() << '\n';

    if (state.street() != Street::Preflop) {
        std::cout << "Stacks: P0="
                  << state.player(0).stack()
                  << " | P1="
                  << state.player(1).stack()
                  << '\n';
    }
}

void printAction(
    std::uint8_t player,
    const Action& action,
    std::int64_t callAmount,
    bool allIn
) {
    std::cout << "Player "
              << static_cast<int>(player)
              << ": ";

    switch (action.type) {
        case ActionType::Fold:
            std::cout << "Fold";
            break;

        case ActionType::Check:
            std::cout << "Check";
            break;

        case ActionType::Call:
            std::cout << "Call " << callAmount;
            break;

        case ActionType::Bet:
            std::cout << "Bet " << action.amount;
            break;

        case ActionType::Raise:
            std::cout << "Raise to " << action.amount;
            break;
    }

    if (allIn) {
        std::cout << " (ALL-IN)";
    }

    std::cout << '\n';
}

void printResult(
    const HandState& state,
    const std::array<std::int64_t, 2>& initialStacks
) {
    std::cout << "\n[RESULT]\n";

    const std::int64_t final0 = state.player(0).stack();
    const std::int64_t final1 = state.player(1).stack();

    const std::int64_t net0 = final0 - initialStacks[0];
    const std::int64_t net1 = final1 - initialStacks[1];

    // Show final board if the hand reached an automatic showdown.
    if (state.board().size() == 5) {
        std::cout << "Final board: ";
        printBoard(state);
        std::cout << '\n';
    }

    if (net0 > 0) {
        std::cout << "Player 0 wins (net +"
                  << net0 << " chips)\n";
    } else if (net1 > 0) {
        std::cout << "Player 1 wins (net +"
                  << net1 << " chips)\n";
    } else {
        std::cout << "Hand ends with no net chip transfer\n";
    }

    std::cout << "Player 0 final stack: "
              << final0 << '\n';

    std::cout << "Player 1 final stack: "
              << final1 << '\n';
}

} // namespace

// --------------------------------------------------
// Main
// --------------------------------------------------

int main() {
    constexpr std::int64_t initialStack = 1000;

    const HandConfig config{50, 100};

    std::mt19937 deckRng{52};
    std::mt19937 agentRng0{200};
    std::mt19937 agentRng1{400};

    HandEngine engine{
        {initialStack, initialStack},
        config,
        0,
        deckRng
    };

    RandomAgent agent0{agentRng0};
    RandomAgent agent1{agentRng1};

    std::array<RandomAgent*, 2> agents{
        &agent0, &agent1
    };

    const std::array<std::int64_t, 2> initialStacks{
        initialStack, initialStack
    };

    std::cout << "=== POKER AI LAB ===\n";
    std::cout << "RandomAgent vs RandomAgent\n\n";

    printInitialState(engine.state(), config);
    printStreet(engine.state());

    constexpr int maxActions = 100;
    int actionCount = 0;

    while (engine.state().phase() == HandPhase::Betting &&
           actionCount < maxActions) {

        const auto actor = engine.state().actingPlayer();

        if (!actor.has_value()) {
            std::cerr << "ERROR: No acting player\n";
            return 1;
        }

        const std::uint8_t player = *actor;

        const auto observation = engine.observe(player);

        const Action action =
            agents[player]->chooseAction(observation);

        // Save information before applying the action.
        const Street previousStreet = engine.state().street();

        const auto& current = engine.state().player(player);
        const auto& opponent = engine.state().player(1 - player);

        const std::int64_t toCall = std::max<std::int64_t>(
            0,
            opponent.committedStreet() - current.committedStreet()
        );

        const std::int64_t callAmount =
            std::min(toCall, current.stack());

        // Apply the action before printing it.
        engine.applyAction(player, action);

        const bool allIn =
            action.type != ActionType::Fold &&
            engine.state().player(player).isAllIn();

        printAction(player, action, callAmount, allIn);

        // Print state only when entering a new betting street.
        if (engine.state().phase() == HandPhase::Betting &&
            engine.state().street() != previousStreet) {

            printStreet(engine.state());
        }

        ++actionCount;
    }

    if (engine.state().phase() != HandPhase::Finished) {
        std::cerr << "ERROR: Hand did not finish\n";
        return 1;
    }

    printResult(engine.state(), initialStacks);

    return 0;
}
