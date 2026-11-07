#pragma once

#include "poker/agent.h"

#include <cstdint>

namespace poker {

struct MatchConfig {
    std::uint64_t games{1000};
    std::uint32_t seed{42};

    std::int64_t startingStack{10000};
    std::int64_t smallBlind{50};
    std::int64_t bigBlind{100};
};

struct MatchResult {
    std::uint64_t gamesPlayed{0};
    std::uint64_t winsA{0};
    std::uint64_t winsB{0};
    std::uint64_t ties{0};

    std::uint64_t totalHands{0};
};

class MatchRunner {
public:
    [[nodiscard]] static MatchResult run(
        Agent& agentA,
        Agent& agentB,
        const MatchConfig& config
    );
};

} // namespace poker