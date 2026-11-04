#include "poker/hand_evaluator.h"

#include <stdexcept>

namespace poker {

using std::invalid_argument;

HandRank HandEvaluator::evaluate(const std::array<Card, 5>& cards) {
    std::array<int, 15> rankCounts{};
    std::array<int, 4> suitCounts{};

    std::array<bool, 52> seen{};

    for (const Card& card : cards) {
        int rank = static_cast<int>(card.rank());
        int suit = static_cast<int>(card.suit());

        int index = (rank - 2) * 4 + suit;

        if (seen[index])
            throw std::invalid_argument("Duplicated cards");

        seen[index] = true;

        ++rankCounts[rank];
        ++suitCounts[suit];
    }

    bool isFlush = false;

    std::uint8_t straightHigh = 0;
    std::uint8_t tripsRank = 0;
    std::uint8_t quadsRank = 0;

    int pairCount = 0;
    std::array<std::uint8_t, 2> pairRanks{};

    HandRank::Kickers singleRanks{};
    std::size_t count = 0;

    for (int suitFreq : suitCounts) {
        if(suitFreq == 5) isFlush = true;
    }

    for (int rank = 14; rank >= 2; --rank) {        
        int frequency = rankCounts[rank];

        if(frequency == 1) singleRanks[count++] = static_cast<std::uint8_t>(rank);

        if (frequency == 4) {
            quadsRank = rank;
        }
        else if (frequency == 3) {
            tripsRank = rank;
        }
        else if (frequency == 2) {
            pairRanks[pairCount++] = rank;
        }

        if(rank >= 6) {
            if( frequency == 1 &&
                rankCounts[rank - 1] == 1 &&
                rankCounts[rank - 2] == 1 &&
                rankCounts[rank - 3] == 1 &&
                rankCounts[rank - 4] == 1 &&
                !straightHigh
            ) {
                straightHigh = rank;
            }
        }
    }

    if( rankCounts[14] == 1 &&
        rankCounts[5] == 1 &&
        rankCounts[4] == 1 &&
        rankCounts[3] == 1 &&
        rankCounts[2] == 1 && 
        !straightHigh
    ) {
        straightHigh = 5;
    }

    HandCategory category;
    HandRank::Kickers kickers{}; 

    if (isFlush && straightHigh) {
        // Straight Flush
        category = HandCategory::StraightFlush;
        kickers = {straightHigh, 0, 0, 0, 0};

    } else if (quadsRank) {
        // Four of a Kind
        category = HandCategory::FourOfAKind;
        kickers = {quadsRank, singleRanks[0], 0, 0, 0};

    } else if (tripsRank && pairCount == 1) {
        // Full House
        category = HandCategory::FullHouse;
        kickers = {tripsRank, pairRanks[0], 0, 0, 0};

    } else if (isFlush) {
        // Flush
        category = HandCategory::Flush;
        kickers = singleRanks;

    } else if (straightHigh) {
        // Straight
        category = HandCategory::Straight;
        kickers = {straightHigh, 0, 0, 0, 0};

    } else if (tripsRank) {
        // Three of a Kind
        category = HandCategory::ThreeOfAKind;
        kickers = {tripsRank, singleRanks[0], singleRanks[1], 0, 0};

    } else if (pairCount == 2) {
        // Two Pair
        category = HandCategory::TwoPair;
        kickers = {pairRanks[0], pairRanks[1], singleRanks[0], 0, 0};

    } else if (pairCount == 1) {
        // One Pair
        category = HandCategory::OnePair;
        kickers = {pairRanks[0], singleRanks[0], singleRanks[1], singleRanks[2], 0};

    } else {
        // High Card
        category = HandCategory::HighCard;
        kickers = singleRanks;
    }

    return HandRank(category, kickers);
}

} // namespace poker