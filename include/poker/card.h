/**
 * @file card.h
 * @brief Representation of a standard playing card.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/rank.h"
#include "poker/suit.h"

namespace poker {

class Card {
public:
    Card(Rank rank, Suit suit);

    [[nodiscard]] Rank rank() const noexcept;
    [[nodiscard]] Suit suit() const noexcept;

    bool operator==(const Card& other) const = default;

private:
    Rank rank_;
    Suit suit_;
};

} // namespace poker