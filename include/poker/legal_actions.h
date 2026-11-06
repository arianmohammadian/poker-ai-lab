/**
 * @file legal_actions.h
 * @brief Defines the legal actions available to a player in a hand.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>
#include <optional>

namespace poker {

struct LegalActions {
    bool canFold{false};
    bool canCheck{false};
    bool canCall{false};
    bool canBet{false};
    bool canRaise{false};

    std::int64_t toCall{0};

    std::int64_t minBet{0};
    std::int64_t maxBet{0};

    std::int64_t minRaise{0};
    std::int64_t maxRaise{0};

    std::optional<std::int64_t> shortAllInBet;
    std::optional<std::int64_t> shortAllInRaise;
};

} // namespace poker