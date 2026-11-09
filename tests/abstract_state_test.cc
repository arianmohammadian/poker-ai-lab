
/**
 * @file abstract_state_test.cc
 * @brief Tests state abstraction and bucket boundaries.
 * @author Arian Mohammadian
 */

#include "poker/abstract_state.h"
#include "poker/hand_engine.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

using namespace poker;

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void testBuckets() {
    std::mt19937 rng{42};
    HandEngine engine({1000, 1000}, {50, 100}, 0, rng);
    PlayerObservation obs = engine.observe(0);

    // Equity buckets and boundaries.
    check(StateAbstractor::abstract(obs, 0.0, 100).equityBucket == 0,
          "Equity 0 must map to bucket 0");
    check(StateAbstractor::abstract(obs, 0.2, 100).equityBucket == 1,
          "Equity 0.2 must map to bucket 1");
    check(StateAbstractor::abstract(obs, 0.6, 100).equityBucket == 3,
          "Equity 0.6 must map to bucket 3");
    check(StateAbstractor::abstract(obs, 1.0, 100).equityBucket == 4,
          "Equity 1 must map to bucket 4");

    // Effective remaining stack.
    obs.stacks = {990, 3000};
    check(StateAbstractor::abstract(obs, 0.5, 100).stackBucket == 0,
          "9.9 BB must map to bucket 0");

    obs.stacks = {2500, 3000};
    check(StateAbstractor::abstract(obs, 0.5, 100).stackBucket == 2,
          "25 BB must map to bucket 2");

    // Pot odds: toCall / (pot + toCall).
    obs.pot = 150;
    obs.legalActions->toCall = 50;
    check(StateAbstractor::abstract(obs, 0.5, 100).potOddsBucket == 2,
          "25% pot odds must map to bucket 2");

    obs.legalActions->toCall = 0;
    obs.pot = 0;
    check(StateAbstractor::abstract(obs, 0.5, 100).potOddsBucket == 0,
          "Zero toCall must map to bucket 0");

    // Button position.
    obs.player = 0;
    obs.button = 0;
    check(StateAbstractor::abstract(obs, 0.5, 100).isButton,
          "Button player must be recognized");

    obs.button = 1;
    check(!StateAbstractor::abstract(obs, 0.5, 100).isButton,
          "Non-button player must be recognized");
}

void testInvalidInputs() {
    std::mt19937 rng{43};
    HandEngine engine({1000, 1000}, {50, 100}, 0, rng);
    PlayerObservation obs = engine.observe(0);

    auto rejects = [&](double equity, std::int64_t bb) {
        try {
            [[maybe_unused]] auto state =
                StateAbstractor::abstract(obs, equity, bb);
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };

    check(rejects(-0.1, 100), "Negative equity must be rejected");
    check(rejects(1.1, 100), "Equity above 1 must be rejected");
    check(rejects(std::numeric_limits<double>::quiet_NaN(), 100),
          "NaN equity must be rejected");
    check(rejects(0.5, 0), "Zero big blind must be rejected");
}

} // namespace

int main() {
    testBuckets();
    testInvalidInputs();

    std::cout << "All AbstractState tests passed!\n";
    return EXIT_SUCCESS;
}
