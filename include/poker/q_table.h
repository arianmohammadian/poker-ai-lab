/**
 * @file q_table.h
 * @brief Stores and updates action-value estimates for tabular poker agents.
 * @author Arian Mohammadian
 */

#pragma once

#include "poker/abstract_state.h"
#include "poker/abstract_action.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace poker {

class QTable {
public:
    static constexpr std::size_t kStates = 640;
    static constexpr std::size_t kActions = 5;
    static constexpr std::size_t kEntries = kStates * kActions;

    [[nodiscard]] double value(
        const AbstractState& state,
        AbstractAction action
    ) const;

    void update(
        const AbstractState& state,
        AbstractAction action,
        double reward
    );

    [[nodiscard]] std::uint64_t visits(
        const AbstractState& state,
        AbstractAction action
    ) const;

private:
    std::array<double, kEntries> values_{};
    std::array<std::uint64_t, kEntries> visits_{};

    [[nodiscard]] static std::size_t index(
        const AbstractState& state,
        AbstractAction action
    );
};

} // namespace poker