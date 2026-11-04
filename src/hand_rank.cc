#include "poker/hand_rank.h"

#include <stdexcept>

namespace poker {

using std::invalid_argument;

namespace {

bool isStraight(const HandRank::Kickers& kickers) {
    if (kickers == HandRank::Kickers{14, 5, 4, 3, 2}) return true;

    for (std::size_t i = 0; i + 1 < kickers.size(); ++i) {
        if(kickers[i] != kickers[i + 1] + 1) return false;
    }

    return true;
}

bool isValidDistinctRanks(const HandRank::Kickers& kickers) {
    return  kickers[0] >= 2 && 
            kickers[1] >= 2 && 
            kickers[2] >= 2 &&
            kickers[3] >= 2 &&
            kickers[4] >= 2 &&
            kickers[0] > kickers[1] &&
            kickers[1] > kickers[2] &&
            kickers[2] > kickers[3] &&
            kickers[3] > kickers[4];
}

bool validateKickers(HandCategory category, const HandRank::Kickers& kickers) {

    // 1. Validaciones generales
    for (std::uint8_t kicker : kickers) {
        if (kicker > 14) return false;
    }

    // 2. Validaciones especificas
    switch (category) {
        case HandCategory::HighCard:
        case HandCategory::Flush:
            return isValidDistinctRanks(kickers) &&
                    !isStraight(kickers);

        case HandCategory::OnePair:
            return  kickers[0] >= 2 && 
                    kickers[1] >= 2 && 
                    kickers[2] >= 2 &&
                    kickers[3] >= 2 &&
                    kickers[0] != kickers[1] &&
                    kickers[0] != kickers[2] &&
                    kickers[0] != kickers[3] &&
                    kickers[1] > kickers[2] && 
                    kickers[2] > kickers[3] && 
                    kickers[4] == 0;

        case HandCategory::TwoPair:
            return  kickers[0] >= 2 && 
                    kickers[1] >= 2 && 
                    kickers[2] >= 2 &&
                    kickers[0] > kickers[1] && 
                    kickers[0] != kickers[2] && 
                    kickers[1] != kickers[2] && 
                    kickers[3] == 0 && 
                    kickers[4] == 0;

        case HandCategory::ThreeOfAKind:
            return  kickers[0] >= 2 && 
                    kickers[1] >= 2 && 
                    kickers[2] >= 2 &&
                    kickers[0] != kickers[1] &&
                    kickers[0] != kickers[2] &&
                    kickers[1] > kickers[2] &&
                    kickers[3] == 0 &&
                    kickers[4] == 0;

        case HandCategory::FullHouse:
        case HandCategory::FourOfAKind:
            return  kickers[0] >= 2 && 
                    kickers[1] >= 2 && 
                    kickers[0] != kickers[1] && 
                    kickers[2] == 0 && 
                    kickers[3] == 0 && 
                    kickers[4] == 0;

        case HandCategory::Straight:
        case HandCategory::StraightFlush:
            return  kickers[0] >= 5 && 
                    kickers[1] == 0 && 
                    kickers[2] == 0 && 
                    kickers[3] == 0 && 
                    kickers[4] == 0;

        default:
            return false;
    }
}

} // namespace

HandRank::HandRank(HandCategory category, Kickers kickers)
    : category_(category), kickers_(kickers)
{
    if(category < HandCategory::HighCard || category > HandCategory::StraightFlush) throw invalid_argument("Invalid category");
    if(!validateKickers(category, kickers)) throw invalid_argument("Invalid kickers");
}

HandCategory HandRank::category() const noexcept {
    return category_;
}

const HandRank::Kickers& HandRank::kickers() const noexcept {
    return kickers_;
}

} // namespace poker