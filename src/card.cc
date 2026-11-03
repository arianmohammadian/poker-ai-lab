#include "poker/card.h"

#include <stdexcept>

namespace poker {

using std::invalid_argument;

Card::Card(Rank rank, Suit suit)
    : rank_(rank), suit_(suit)
{
    if(rank < Rank::Two || rank > Rank::Ace) throw invalid_argument("Invalid rank");
    if(suit < Suit::Clubs || suit > Suit::Spades) throw invalid_argument("Invalid suit");
}

Rank Card::rank() const noexcept {
    return rank_;
}

Suit Card::suit() const noexcept {
    return suit_;
}

} // namespace poker