#include "poker/deck.h"

#include <stdexcept>
#include <algorithm>

namespace poker {

using std::out_of_range;

Deck::Deck() {
    cards_.reserve(kDeckSize);
    
    for(int rank = 2; rank < 15; ++rank) {
        for(int suit = 0; suit < 4; ++suit) {
            Rank r = static_cast<Rank>(rank);
            Suit s = static_cast<Suit>(suit);
            cards_.emplace_back(r, s);
        }
    }
}

void Deck::shuffle(std::mt19937& rng) {
    std::shuffle(cards_.begin(), cards_.end(), rng);
    next_ = 0;
} 

void Deck::reset() noexcept {
    next_ = 0;
}

Card Deck::draw() {
    if(remaining() == 0) throw out_of_range("Deck is empty");

    return cards_[next_++];
}

std::size_t Deck::remaining() const noexcept {
    return kDeckSize - next_;
}


}