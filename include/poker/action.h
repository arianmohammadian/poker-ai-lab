/**
 * @file action.h
 * @brief Defines player actions in Heads-Up No-Limit Texas Hold'em.
 * @author Arian Mohammadian
 */
#pragma once

#include <cstdint>

namespace poker {

enum class ActionType : std::uint8_t {
    Fold,
    Check,
    Call,
    Bet,
    Raise
};

struct Action {
    ActionType type;
    std::int64_t amount{0};
};

} // namespace poker