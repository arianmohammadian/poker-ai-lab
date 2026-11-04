#include "poker/equity_calculator.h"

#include <stdexcept>

namespace poker {

namespace {

using std::invalid_argument;

void checkAndMark(const Card& card, std::array<bool, 52>& seen) {
    int rank = static_cast<int>(card.rank);
    int suit = static_cast<int>(card.suit);

    int cardIDX = (rank - 2) * 4 + suit;

    if(seen[cardIDX]) throw invalid_argument("Duplicated cards");
    else seen[cardIDX] = true;
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

} // namespace

} // namespace poker