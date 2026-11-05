#include "poker/equity_calculator.h"
#include "poker/hand_evaluator.h"

#include <stdexcept>
#include <random>
#include <algorithm>

namespace poker {

namespace {

using std::invalid_argument;

int cardId(const Card& card) {
    int rank = static_cast<int>(card.rank());
    int suit = static_cast<int>(card.suit());

    return (rank - 2) * 4 + suit;
}

std::vector<Card> availableCards(const EquityInput& input) {
    const auto& player1 = input.player1;
    const auto& player2 = input.player2;
    const auto& board = input.board;

    std::vector<Card> available;
    available.reserve(52 - 4 - board.size());

    std::array<bool, 52> seen{};

    for(const auto& card : player1) seen[cardId(card)] = true;
    for(const auto& card : player2) seen[cardId(card)] = true;
    for(const auto& card : board) seen[cardId(card)] = true;

    for (int rank = 2; rank <= 14; ++rank) {
        for (int suit = 0; suit < 4; ++suit) {
            int id = (rank - 2) * 4 + suit;

            if (!seen[id]) available.emplace_back(static_cast<Rank>(rank), static_cast<Suit>(suit));
        }
    }

    return available;
}

void evaluateShowdown(
    const std::array<Card, 2>& player1,
    const std::array<Card, 2>& player2,
    const std::array<Card, 5>& board,
    EquityResult& result
) {
    const std::array<Card, 7> cardsp1 = {
        board[0],
        board[1],
        board[2],
        board[3],
        board[4],
        player1[0],
        player1[1]
    };

    const std::array<Card, 7> cardsp2 = {
        board[0],
        board[1],
        board[2],
        board[3],
        board[4],
        player2[0],
        player2[1]
    };

    HandRank handp1 = HandEvaluator::evaluate(cardsp1);
    HandRank handp2 = HandEvaluator::evaluate(cardsp2);

    if(handp1 > handp2) result.player1Wins++;
    else if(handp1 < handp2) result.player2Wins++;
    else result.ties++;
}

void checkAndMark(const Card& card, std::array<bool, 52>& seen) {
    int id = cardId(card);

    if(seen[id]) throw invalid_argument("Duplicated cards");
    
    seen[id] = true;
}

void validateInput(const EquityInput& input) {
    const auto& player1 = input.player1;
    const auto& player2 = input.player2;
    const auto& board = input.board;

    if( board.size() != 0 &&
        board.size() != 3 &&
        board.size() != 4 &&
        board.size() != 5 ) {
            throw invalid_argument("Board size is not correct");
    }

    std::array<bool, 52> seen{};

    for(const Card& card : player1) checkAndMark(card, seen);
    for(const Card& card : player2) checkAndMark(card, seen);

    for(const Card& card : board) checkAndMark(card, seen);
}

void enumerateBoards(
    const EquityInput& input,
    const std::vector<Card>& available,
    std::vector<Card>& currentBoard,
    std::size_t start,
    EquityResult& result
) {
    if(currentBoard.size() == 5) {
        std::array<Card, 5> board = {
            currentBoard[0],
            currentBoard[1],
            currentBoard[2],
            currentBoard[3],
            currentBoard[4]
        };

        evaluateShowdown(input.player1, input.player2, board, result);
        return;
    }

    // Increasing indices ensure each board combination is generated exactly once.

    for (std::size_t i = start; i < available.size(); ++i) {
        currentBoard.push_back(available[i]);
        enumerateBoards(input, available, currentBoard, i + 1, result);
        currentBoard.pop_back();
    }
}

} // namespace

EquityResult EquityCalculator::calculateExact(const EquityInput& input) {
    validateInput(input);

    EquityResult result;

    std::vector<Card> currentBoard = input.board;

    enumerateBoards(input, availableCards(input), currentBoard, 0, result);

    return result;
}

EquityResult EquityCalculator::calculateMonteCarlo(
    const EquityInput& input,
    std::uint64_t samples,
    std::uint64_t seed
) {
    validateInput(input);

    if (samples == 0)
        throw std::invalid_argument("Sample count must be positive");

    std::mt19937_64 rng(seed);

    std::vector<Card> available = availableCards(input);

    const std::size_t missing = 5 - input.board.size();

    EquityResult result;

    std::vector<Card> currentBoard = input.board;
    currentBoard.reserve(5);

    for(std::uint64_t t = 0; t < samples; t++) {
        for (std::size_t i = 0; i < missing; ++i) {
            std::uniform_int_distribution<std::size_t> dist(
                i, available.size() - 1
            );

            std::size_t j = dist(rng);

            std::swap(available[i], available[j]);

            currentBoard.push_back(available[i]);
        }

        const std::array<Card, 5> completeBoard = {
            currentBoard[0],
            currentBoard[1],
            currentBoard[2],
            currentBoard[3],
            currentBoard[4]
        };

        evaluateShowdown(input.player1, input.player2, completeBoard, result);

        for (std::size_t i = 0; i < missing; ++i) {
            currentBoard.pop_back();
        }
    }
    
    return result;
}

} // namespace poker