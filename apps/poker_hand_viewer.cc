
/**
 * @file poker_hand_viewer.cc
 * @brief Displays autonomous Heads-Up NLHE games in the terminal.
 * @author Arian Mohammadian
 */

#include "poker/game_engine.h"
#include "poker/random_agent.h"
#include "poker/hand_evaluator.h"
#include "poker/hand_rank.h"
#include "poker/hand_category.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>

using poker::Action;
using poker::ActionType;
using poker::Card;
using poker::GameEngine;
using poker::HandConfig;
using poker::HandPhase;
using poker::HandState;
using poker::HandEvaluator;
using poker::HandRank;
using poker::HandCategory;
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

void printCards(
    const std::vector<Card>& cards,
    std::size_t first,
    std::size_t last
) {
    for (std::size_t i = first; i < last; ++i) {
        if (i > first) std::cout << ' ';
        printCard(cards[i]);
    }
}

const char* streetName(Street street) {
    switch (street) {
        case Street::Preflop: return "PREFLOP";
        case Street::Flop:    return "FLOP";
        case Street::Turn:    return "TURN";
        case Street::River:   return "RIVER";
    }

    throw std::invalid_argument("Invalid street");
}

// --------------------------------------------------
// Hand presentation
// --------------------------------------------------

void printInitialState(
    const HandState& state,
    const HandConfig& config,
    std::uint64_t handNumber,
    const std::array<std::int64_t, 2>& startingStacks
) {
    std::cout << "\n========== HAND #"
              << handNumber
              << " ==========\n";

    std::cout << "Button: Player "
              << static_cast<int>(state.button())
              << '\n';

    std::cout << "Blinds: "
              << config.smallBlind << '/'
              << config.bigBlind << '\n';

    for (int i = 0; i < 2; ++i) {
        std::cout << "Player " << i << ": ";

        const auto& cards = state.player(i).holeCards();

        printCard(cards[0]);
        std::cout << ' ';
        printCard(cards[1]);

        std::cout << "    Starting stack: "
                  << startingStacks[i]
                  << '\n';
    }
}

void printStreet(const HandState& state) {
    std::cout << "\n["
              << streetName(state.street())
              << "]";

    if (!state.board().empty()) {
        std::cout << ' ';
        printCards(
            state.board(),
            0,
            state.board().size()
        );
    }

    std::cout << "\nPot: "
              << state.pot()
              << '\n';

    if (state.street() != Street::Preflop) {
        std::cout << "Stacks: P0="
                  << state.player(0).stack()
                  << " | P1="
                  << state.player(1).stack()
                  << '\n';
    }
}

// Shows cards dealt automatically after the last
// betting action, without repeating earlier streets.
void printRunout(
    const HandState& state,
    std::size_t previousBoardSize
) {
    const auto& board = state.board();

    if (board.size() <= previousBoardSize)
        return;

    std::cout << "\n[ALL-IN RUNOUT]\n";

    if (previousBoardSize < 3 && board.size() >= 3) {
        std::cout << "Flop: ";
        printCards(board, 0, 3);
        std::cout << '\n';
    }

    if (previousBoardSize < 4 && board.size() >= 4) {
        std::cout << "Turn: ";
        printCard(board[3]);
        std::cout << '\n';
    }

    if (previousBoardSize < 5 && board.size() >= 5) {
        std::cout << "River: ";
        printCard(board[4]);
        std::cout << '\n';
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


const char* rankName(std::uint8_t rank) {
    switch (rank) {
        case 2:  return "Twos";
        case 3:  return "Threes";
        case 4:  return "Fours";
        case 5:  return "Fives";
        case 6:  return "Sixes";
        case 7:  return "Sevens";
        case 8:  return "Eights";
        case 9:  return "Nines";
        case 10: return "Tens";
        case 11: return "Jacks";
        case 12: return "Queens";
        case 13: return "Kings";
        case 14: return "Aces";
    }

    throw std::invalid_argument("Invalid rank");
}

const char* categoryName(HandCategory category) {
    switch (category) {
        case HandCategory::HighCard:     return "High Card";
        case HandCategory::OnePair:      return "One Pair";
        case HandCategory::TwoPair:      return "Two Pair";
        case HandCategory::ThreeOfAKind: return "Three of a Kind";
        case HandCategory::Straight:     return "Straight";
        case HandCategory::Flush:        return "Flush";
        case HandCategory::FullHouse:    return "Full House";
        case HandCategory::FourOfAKind:  return "Four of a Kind";
        case HandCategory::StraightFlush:return "Straight Flush";
    }

    throw std::invalid_argument("Invalid hand category");
}

void printHandRank(const HandRank& rank) {
    const auto category = rank.category();
    const auto& kickers = rank.kickers();

    std::cout << categoryName(category);

    switch (category) {
        case HandCategory::OnePair:
        case HandCategory::ThreeOfAKind:
        case HandCategory::FourOfAKind:
            std::cout << " (" << rankName(kickers[0]) << ")";
            break;

        case HandCategory::TwoPair:
            std::cout << " (" << rankName(kickers[0])
                      << " and " << rankName(kickers[1]) << ")";
            break;

        case HandCategory::FullHouse:
            std::cout << " (" << rankName(kickers[0])
                      << " full of " << rankName(kickers[1]) << ")";
            break;

        case HandCategory::Straight:
        case HandCategory::StraightFlush:
            std::cout << " ("
                      << (kickers[0] == 5 ? "Five" :
                          kickers[0] == 14 ? "Ace" :
                          rankName(kickers[0]))
                      << "-high)";
            break;

        case HandCategory::HighCard:
        case HandCategory::Flush:
            std::cout << " (" << rankName(kickers[0]) << "-high)";
            break;
    }
}

HandRank evaluatePlayer(const HandState& state, std::uint8_t player) {
    const auto& hole = state.player(player).holeCards();
    const auto& board = state.board();

    if (board.size() != 5) {
        throw std::logic_error("Showdown requires five community cards");
    }

    std::array<Card, 7> cards{
        hole[0], hole[1],
        board[0], board[1], board[2], board[3], board[4]
    };

    return HandEvaluator::evaluate(cards);
}


// --------------------------------------------------
// Results
// --------------------------------------------------


void printHandResult(
    const HandState& state,
    const std::array<std::int64_t, 2>& startingStacks
) {
    std::cout << "\n[HAND RESULT]\n";

    // Print the community cards dealt during the hand.
    std::cout << "Board: ";

    if (state.board().empty()) {
        std::cout << "(no community cards)";
    } else {
        printCards(state.board(), 0, state.board().size());
    }

    std::cout << "\n\n";

    const bool folded0 = state.player(0).hasFolded();
    const bool folded1 = state.player(1).hasFolded();
    const bool endedByFold = folded0 || folded1;

    // --------------------------------------------------
    // Fold
    // --------------------------------------------------

    if (endedByFold) {
        const int winner = folded0 ? 1 : 0;

        for (int i = 0; i < 2; ++i) {
            const auto& hole = state.player(i).holeCards();

            std::cout << "Player " << i << ": ";

            printCard(hole[0]);
            std::cout << ' ';
            printCard(hole[1]);

            std::cout << " -> ";

            if (state.player(i).hasFolded()) {
                std::cout << "Fold";
            } else {
                std::cout << "Wins by Fold";
            }

            std::cout << '\n';
        }

        std::cout << "\nWinner: Player "
                  << winner << " (Fold)\n";
    }

    // --------------------------------------------------
    // Showdown
    // --------------------------------------------------

    else {
        const HandRank rank0 = evaluatePlayer(state, 0);
        const HandRank rank1 = evaluatePlayer(state, 1);

        const std::array<HandRank, 2> ranks{
            rank0, rank1
        };

        for (int i = 0; i < 2; ++i) {
            const auto& hole = state.player(i).holeCards();

            std::cout << "Player " << i << ": ";

            printCard(hole[0]);
            std::cout << ' ';
            printCard(hole[1]);

            std::cout << " -> ";
            printHandRank(ranks[i]);
            std::cout << '\n';
        }

        if (rank0 > rank1) {
            std::cout << "\nWinner: Player 0 (";
            printHandRank(rank0);
            std::cout << ")\n";
        } else if (rank1 > rank0) {
            std::cout << "\nWinner: Player 1 (";
            printHandRank(rank1);
            std::cout << ")\n";
        } else {
            std::cout << "\nResult: Split Pot\n";
        }
    }

    // --------------------------------------------------
    // Final stacks and net winnings
    // --------------------------------------------------

    std::cout << '\n';

    for (int i = 0; i < 2; ++i) {
        const std::int64_t finalStack =
            state.player(i).stack();

        const std::int64_t net =
            finalStack - startingStacks[i];

        std::cout << "Player " << i
                  << ": " << finalStack
                  << " (";

        if (net >= 0) {
            std::cout << '+';
        }

        std::cout << net << ")\n";
    }
}


void printGameResult(const GameEngine& game) {
    const HandState& state = game.currentHand().state();

    const std::int64_t stack0 = state.player(0).stack();
    const std::int64_t stack1 = state.player(1).stack();

    std::cout << "\n========== GAME RESULT ==========\n";

    std::cout << "Hands played: "
              << game.handNumber() << '\n';

    std::cout << "Final stacks:\n";
    std::cout << "  Player 0: " << stack0 << '\n';
    std::cout << "  Player 1: " << stack1 << '\n';

    if (game.isFinished()) {
        if (stack0 == 0) {
            std::cout << "Winner: Player 1\n";
        } else if (stack1 == 0) {
            std::cout << "Winner: Player 0\n";
        }
    } else {
        std::cout << "Game stopped: hand limit reached\n";

        if (stack0 > stack1) {
            std::cout << "Chip leader: Player 0\n";
        } else if (stack1 > stack0) {
            std::cout << "Chip leader: Player 1\n";
        } else {
            std::cout << "Chip stacks are tied\n";
        }
    }

    std::cout << "=================================\n";
}

} // namespace

// --------------------------------------------------
// Main
// --------------------------------------------------

int main() {
    constexpr std::int64_t initialStack = 1000;
    constexpr std::uint64_t maxHands = 100;
    constexpr int maxActionsPerHand = 100;

    const HandConfig config{50, 100};

    std::mt19937 deckRng{52};
    std::mt19937 agentRng0{200};
    std::mt19937 agentRng1{400};

    GameEngine game{
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

    // Store stacks BEFORE each hand, independently
    // of the state inside HandEngine.
    std::array<std::int64_t, 2> startingStacks{
        initialStack, initialStack
    };

    std::cout << "=== POKER AI LAB ===\n";
    std::cout << "RandomAgent vs RandomAgent\n";
    std::cout << "Maximum hands: " << maxHands << '\n';

    // The first hand may already be finished.
    // Therefore, do not require !game.isFinished()
    // before displaying its result.
    while (true) {
        const HandState& initialState =
            game.currentHand().state();

        printInitialState(
            initialState,
            config,
            game.handNumber(),
            startingStacks
        );

        if (initialState.phase() == HandPhase::Betting) {
            printStreet(initialState);
        } else {
            // A hand can finish automatically when
            // the blinds put a player all-in.
            std::cout << "\n[AUTOMATIC ALL-IN]\n";

            printRunout(initialState, 0);
        }

        int actionCount = 0;

        // --------------------------------------------------
        // Betting loop
        // --------------------------------------------------

        while (game.currentHand().state().phase() ==
               HandPhase::Betting) {

            if (actionCount >= maxActionsPerHand) {
                std::cerr
                    << "ERROR: Hand exceeded action limit\n";
                return EXIT_FAILURE;
            }

            const HandState& state =
                game.currentHand().state();

            const auto actor = state.actingPlayer();

            if (!actor.has_value()) {
                std::cerr << "ERROR: No acting player\n";
                return EXIT_FAILURE;
            }

            const std::uint8_t player = *actor;

            const auto observation =
                game.currentHand().observe(player);

            const Action action =
                agents[player]->chooseAction(observation);

            const Street previousStreet = state.street();
            const std::size_t previousBoardSize =
                state.board().size();

            const auto& current = state.player(player);
            const auto& opponent = state.player(1 - player);

            const std::int64_t toCall =
                std::max<std::int64_t>(
                    0,
                    opponent.committedStreet() -
                    current.committedStreet()
                );

            const std::int64_t callAmount =
                std::min(toCall, current.stack());

            game.applyAction(player, action);

            const HandState& updatedState =
                game.currentHand().state();

            const bool allIn =
                action.type != ActionType::Fold &&
                action.type != ActionType::Check &&
                updatedState.player(player).isAllIn();

            printAction(
                player,
                action,
                callAmount,
                allIn
            );

            if (updatedState.phase() == HandPhase::Betting) {
                if (updatedState.street() != previousStreet) {
                    printStreet(updatedState);
                }
            } else {
                // The hand may have automatically advanced
                // through multiple streets to showdown.
                printRunout(
                    updatedState,
                    previousBoardSize
                );
            }

            ++actionCount;
        }

        const HandState& finishedState =
            game.currentHand().state();

        if (finishedState.phase() != HandPhase::Finished) {
            std::cerr << "ERROR: Hand did not finish\n";
            return EXIT_FAILURE;
        }

        printHandResult(finishedState, startingStacks);

        // Capture final stacks BEFORE replacing HandEngine.
        startingStacks = {
            finishedState.player(0).stack(),
            finishedState.player(1).stack()
        };

        if (game.isFinished() ||
            game.handNumber() >= maxHands) {
            break;
        }

        game.nextHand();
    }

    printGameResult(game);

    return EXIT_SUCCESS;
}
