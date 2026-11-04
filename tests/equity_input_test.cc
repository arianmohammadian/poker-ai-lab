/**
 * @file equity_input_test.cc
 * @brief Construction tests for Heads-Up equity input data.
 * @author Arian Mohammadian
 */

#include "poker/card.h"
#include "poker/equity_input.h"

#include <array>
#include <cstddef>
#include <initializer_list>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

using poker::Card;
using poker::EquityInput;
using poker::Rank;
using poker::Suit;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

Card card(int rank, int suit) {
    return Card(static_cast<Rank>(rank), static_cast<Suit>(suit));
}

void testSupportedBoardSizes() {
    const auto player1 = std::array<Card, 2>{card(14, 0), card(14, 1)};
    const auto player2 = std::array<Card, 2>{card(13, 0), card(13, 1)};
    const std::vector<Card> community{
        card(2, 0), card(7, 1), card(11, 2), card(9, 3), card(3, 0)
    };

    for (const std::size_t boardSize : {std::size_t{0}, std::size_t{3},
                                        std::size_t{4}, std::size_t{5}}) {
        const std::vector<Card> board(community.begin(),
                                      community.begin() + boardSize);
        const EquityInput input{player1, player2, board};
        check(input.player1.size() == 2, "Player 1 has two cards");
        check(input.player2.size() == 2, "Player 2 has two cards");
        check(input.board.size() == boardSize, "Input preserves board size");
    }
}

} // namespace

int main() {
    testSupportedBoardSizes();
    std::cout << "All EquityInput construction tests passed.\n";
    return EXIT_SUCCESS;
}
