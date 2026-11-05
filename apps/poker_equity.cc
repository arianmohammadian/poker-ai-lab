/**
 * @file poker_equity.cc
 * @brief Interactive CLI for Heads-Up Texas Hold'em equity.
 * @author Arian Mohammadian
 */

#include "poker/card.h"
#include "poker/equity_calculator.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>
#include <iomanip>

namespace {

using poker::Card;
using poker::Rank;
using poker::Suit;
using poker::EquityInput;
using poker::EquityResult;
using poker::EquityCalculator;

using std::invalid_argument;

Card parseCard(const std::string& text) {
    // 1. Validate text length
    if(text.size() != 2) throw invalid_argument("Invalid card format");

    // 2. Convert text[0] to Rank
    Rank rank = Rank::Two; 

    switch (text[0]) {
        case 'A':
            rank = Rank::Ace;
            break;

        case 'K':
            rank = Rank::King;
            break;

        case 'Q':
            rank = Rank::Queen;
            break;

        case 'J':
            rank = Rank::Jack;
            break;

        case 'T':
            rank = Rank::Ten;
            break;

        default:
            if(text[0] < '2' || text[0] > '9') throw invalid_argument("Invalid card rank");
            else rank = static_cast<Rank>(text[0] - '0');
    }

    // 3. Convert text[1] to Suit
    Suit suit = Suit::Clubs; 

    switch(text[1]) {
        case 'c':
            suit = Suit::Clubs;
            break;

        case 'd':
            suit = Suit::Diamonds;
            break;

        case 'h':
            suit = Suit::Hearts;
            break;

        case 's':
            suit = Suit::Spades;
            break;

        default:
            throw invalid_argument("Invalid Card suit");
    }

    // 4. Construct and return Card
    return Card(rank, suit);
}

std::array<Card, 2> readHand(const std::string& line) {
    std::istringstream stream(line);

    std::string first, second, extra;

    // Read exactly two cards
    if (!(stream >> first >> second)) {
        throw std::invalid_argument("A hand must contain exactly two cards");
    }

    // Reject additional cards or tokens
    if (stream >> extra) {
        throw std::invalid_argument("A hand must contain exactly two cards");
    }

    return {
        parseCard(first),
        parseCard(second)
    };
}

std::vector<Card> readBoard(const std::string& line) {
    std::istringstream stream(line);

    std::vector<Card> board;
    std::string token;

    // Parse all cards
    while (stream >> token) {
        board.push_back(parseCard(token));
    }

    // Validate board size
    if (board.size() != 0 &&
        board.size() != 3 &&
        board.size() != 4 &&
        board.size() != 5) {
        throw std::invalid_argument("Board must contain 0, 3, 4 or 5 cards");
    }

    return board;
}

} // namespace

int main() {
    std::cout << "================================\n";
    std::cout << "         POKER AI LAB\n";
    std::cout << "     HU Equity Calculator\n";
    std::cout << "================================\n\n";

    while (true) {
        try {
            std::string line;

            // Read player 1 hand
            std::cout << "Player 1 (e.g. Ah As): ";
            if (!std::getline(std::cin, line)) break;
            auto player1 = readHand(line);

            // Read player 2 hand
            std::cout << "Player 2 (e.g. Kh Ks): ";
            if (!std::getline(std::cin, line)) break;
            auto player2 = readHand(line);

            // Read community cards
            std::cout << "Board (empty / 3 / 4 / 5 cards): ";
            if (!std::getline(std::cin, line)) break;
            auto board = readBoard(line);

            // Calculate exact equity
            poker::EquityInput input{player1, player2, board};

            std::cout << "\nCalculating exact equity...\n";

            poker::EquityResult result =
                poker::EquityCalculator::calculateExact(input);

            // Display results
            std::cout << "\n--------------------------------\n";

            std::cout << std::fixed << std::setprecision(2);

            std::cout << "Player 1 equity: "
                      << result.player1Equity() * 100.0 << "%\n";

            std::cout << "Player 2 equity: "
                      << result.player2Equity() * 100.0 << "%\n";

            std::cout << "\nPlayer 1 wins: " << result.player1Wins << '\n';
            std::cout << "Player 2 wins: " << result.player2Wins << '\n';
            std::cout << "Ties:          " << result.ties << '\n';
            std::cout << "Scenarios:     " << result.total() << '\n';

            std::cout << "--------------------------------\n";

        } catch (const std::exception& e) {
            std::cerr << "\nError: " << e.what() << '\n';
        }

        // Ask whether to continue
        std::cout << "\nCalculate another hand? (y/n): ";

        std::string answer;
        if (!std::getline(std::cin, answer) || answer != "y") {
            break;
        }

        std::cout << '\n';
    }

    std::cout << "\nThanks for using Poker AI Lab!\n";
    return 0;
}