/**
 * @file deck.h
 * @brief Defines a standard 52-card deck with shuffling and dealing operations.
 * @author Arian Mohammadian
 */
#pragma once

#include "poker/card.h"

#include <vector>
#include <cstddef>
#include <random>

namespace poker {

class Deck {
public:
    static constexpr std::size_t kDeckSize = 52;

    Deck();

    Deck(const Deck&) = default;
    Deck& operator=(const Deck&) = default;

    void shuffle(std::mt19937& rng);
    void reset() noexcept;

    [[nodiscard]] Card draw();
    [[nodiscard]] std::size_t remaining() const noexcept;

private:
    std::vector<Card> cards_;
    std::size_t next_ = 0;
};

} // namespace poker