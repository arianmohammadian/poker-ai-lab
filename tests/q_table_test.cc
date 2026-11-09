/**
 * @file q_table_test.cc
 * @brief Tests Q-value updates, independent indexing, and checkpoint integrity.
 * @author Arian Mohammadian
 */

#include "poker/q_table.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace poker;

namespace {

void check(bool ok, const char* message) {
    if (!ok) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

constexpr AbstractState kState{Street::Preflop, 3, 1, 2, true};

void testUpdatesAndIndependence() {
    QTable q;
    const auto a = AbstractAction::CheckCall;
    const auto b = AbstractAction::Pot;
    const AbstractState other{Street::River, 3, 1, 2, true};

    check(q.value(kState, a) == 0.0 && q.visits(kState, a) == 0,
          "Q values and visits must start at zero");
    q.update(kState, a, 2.0);
    q.update(kState, a, -1.0);
    q.update(kState, a, 3.0);

    check(q.visits(kState, a) == 3, "Visit count must be three");
    check(std::abs(q.value(kState, a) - 4.0 / 3.0) < 1e-12,
          "Q must equal the incremental sample mean");
    check(q.visits(kState, b) == 0 && q.visits(other, a) == 0,
          "Different state/action pairs must remain independent");
}

void testInvalidInputs() {
    QTable q;
    bool rejected = false;
    try {
        q.update(kState, AbstractAction::CheckCall,
                 std::numeric_limits<double>::quiet_NaN());
    } catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "NaN rewards must be rejected");

    rejected = false;
    try {
        [[maybe_unused]] auto v = q.value(
            {Street::Preflop, 250, 0, 0, false}, AbstractAction::Fold);
    } catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Invalid state bucket must be rejected");
}

void testCheckpointAndRollback() {
    const auto path = std::filesystem::temp_directory_path() /
                      "poker_ai_lab_q_table_test.qtable";
    QTable original;
    original.update(kState, AbstractAction::CheckCall, 1.25);
    original.update(kState, AbstractAction::CheckCall, -0.5);
    original.save(path.string());

    QTable restored;
    restored.load(path.string());
    check(restored.visits(kState, AbstractAction::CheckCall) == 2,
          "Restored visit count must match");
    check(restored.value(kState, AbstractAction::CheckCall) == 0.375,
          "Restored Q value must match exactly");

    // Corrupted checkpoint must leave current table intact.
    { std::ofstream output(path, std::ios::trunc); output << "BROKEN 1 3200\n"; }
    bool rejected = false;
    try { restored.load(path.string()); }
    catch (const std::runtime_error&) { rejected = true; }
    check(rejected, "Corrupted checkpoint must be rejected");
    check(restored.visits(kState, AbstractAction::CheckCall) == 2 &&
          restored.value(kState, AbstractAction::CheckCall) == 0.375,
          "Failed load must preserve current table");
    std::filesystem::remove(path);
}

} // namespace

int main() {
    testUpdatesAndIndependence();
    testInvalidInputs();
    testCheckpointAndRollback();
    std::cout << "All QTable tests passed!\n";
    return EXIT_SUCCESS;
}
