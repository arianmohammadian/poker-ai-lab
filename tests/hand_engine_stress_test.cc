/**
 * @file hand_engine_stress_test.cc
 * @brief Reproducible randomized integration checks for Poker AI Lab v0.3.
 *
 * Run as a separate CTest executable. No test framework dependency.
 * Tests actual HandEngine transitions, not a reference poker implementation.
 */
#include "poker/hand_engine.h"
#include "poker/action.h"
#include "poker/hand_phase.h"

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace {
using poker::Action;
using poker::ActionType;
using poker::HandEngine;
using poker::HandConfig;
using poker::HandPhase;

constexpr int kHands = 10000;
constexpr int kMaxActions = 128;

void require(bool ok, int hand, int action, const char* msg) {
    if (!ok) {
        throw std::runtime_error("hand=" + std::to_string(hand) +
                                 " action=" + std::to_string(action) + ": " + msg);
    }
}

void invariants(const HandEngine& e, std::int64_t total, int h, int step) {
    const auto& s = e.state();
    const auto& p0 = s.player(0);
    const auto& p1 = s.player(1);
    require(p0.stack() >= 0 && p1.stack() >= 0, h, step, "negative stack");
    require(s.pot() >= 0, h, step, "negative pot");
    require(p0.stack() + p1.stack() + s.pot() == total,
            h, step, "chips are not conserved");
    require(s.payout(0) >= 0 && s.payout(1) >= 0,
            h, step, "negative payout");
    require(s.payout(0) + s.payout(1) <=
                p0.committedTotal() + p1.committedTotal(),
            h, step, "overpaid pot");
    require(p0.committedStreet() >= 0 && p1.committedStreet() >= 0,
            h, step, "negative street commitment");
    require(p0.committedTotal() >= p0.committedStreet() &&
                p1.committedTotal() >= p1.committedStreet(),
            h, step, "street commitments exceed total commitments");

    const auto n = s.board().size();
    require(n == 0 || n == 3 || n == 4 || n == 5,
            h, step, "invalid board size");
    if (s.phase() == HandPhase::Betting) {
        require(s.actingPlayer().has_value(), h, step, "Betting without actor");
        require(s.actionsToClose() > 0, h, step, "Betting without pending actions");
        require(*s.actingPlayer() < 2, h, step, "invalid actor");
        require(!s.player(*s.actingPlayer()).hasFolded(),
                h, step, "folded actor");
        require(!s.player(*s.actingPlayer()).isAllIn(),
                h, step, "all-in actor");
    }
    if (s.phase() == HandPhase::Finished) {
        require(!s.actingPlayer().has_value(), h, step, "Finished with actor");
        require(s.actionsToClose() == 0, h, step, "Finished with pending actions");
        require(s.pot() == 0, h, step, "Finished with unsettled pot");
        require(p0.stack() + p1.stack() == total,
                h, step, "Finished with missing chips");
        if (!p0.hasFolded() && !p1.hasFolded())
            require(n == 5, h, step, "showdown missing board");
    }
}

// Small bounded hash of publicly observable numeric state, for reproducibility.
void mix(std::uint64_t& hash, std::uint64_t v) {
    hash ^= v + 0x9e3779b97f4a7c15ULL + (hash << 6U) + (hash >> 2U);
}

std::uint64_t simulate(std::uint32_t seed) {
    std::mt19937 prng{seed};
    std::uniform_int_distribution<int> stackDist{1, 2500};
    std::uniform_int_distribution<int> choiceDist{0, 99};
    std::uint64_t fingerprint = 0xcbf29ce484222325ULL;

    for (int h = 0; h < kHands; ++h) {
        const std::array<std::int64_t, 2> stacks{stackDist(prng), stackDist(prng)};
        const std::uint8_t button = static_cast<std::uint8_t>(prng() % 2U);
        const std::int64_t initialTotal = stacks[0] + stacks[1];
        std::mt19937 deckRng{prng()};
        HandEngine engine{stacks, HandConfig{50, 100}, button, deckRng};

        int steps = 0;
        invariants(engine, initialTotal, h, steps);
        while (engine.state().phase() != HandPhase::Finished) {
            require(steps < kMaxActions, h, steps, "hand did not terminate");
            const auto& s = engine.state();
            const auto actor = *s.actingPlayer();
            const auto& me = s.player(actor);
            const auto& opp = s.player(1 - actor);
            const auto owed = opp.committedStreet() - me.committedStreet();
            require(owed >= 0, h, steps, "actor has more committed chips");

            Action a{owed > 0 ? ActionType::Call : ActionType::Check, 0};
            const int choice = choiceDist(prng);
            if (choice < 10) {
                a = {ActionType::Fold, 0};
            } else if (choice < 30 && !opp.isAllIn()) {
                // Try a legal all-in bet/raise when it can exceed the current bet.
                const std::int64_t target = me.committedStreet() + me.stack();
                if (owed == 0 && me.committedStreet() == 0 &&
                    opp.committedStreet() == 0) {
                    a = {ActionType::Bet, target};
                } else if (target > opp.committedStreet()) {
                    a = {ActionType::Raise, target};
                }
            } else if (choice < 50 && !opp.isAllIn()) {
                // Prefer a minimum legal bet / full raise when affordable.
                if (owed == 0 && me.committedStreet() == 0 &&
                    opp.committedStreet() == 0 && me.stack() >= 100) {
                    a = {ActionType::Bet, 100};
                } else if (me.committedStreet() != 0 ||
                           opp.committedStreet() != 0) {
                    const auto target = opp.committedStreet() + s.lastFullRaise();
                    if (target > opp.committedStreet() &&
                        target - me.committedStreet() <= me.stack()) {
                        a = {ActionType::Raise, target};
                    }
                }
            }
            engine.applyAction(actor, a);
            ++steps;
            invariants(engine, initialTotal, h, steps);
        }
        const auto& s = engine.state();
        mix(fingerprint, static_cast<std::uint64_t>(steps));
        mix(fingerprint, static_cast<std::uint64_t>(s.player(0).stack()));
        mix(fingerprint, static_cast<std::uint64_t>(s.player(1).stack()));
        mix(fingerprint, static_cast<std::uint64_t>(s.payout(0)));
        mix(fingerprint, static_cast<std::uint64_t>(s.payout(1)));
        mix(fingerprint, static_cast<std::uint64_t>(s.board().size()));
    }
    return fingerprint;
}
} // namespace

int main() {
    try {
        const auto a = simulate(202603U);
        const auto b = simulate(202603U);
        if (a != b)
            throw std::runtime_error("Identical seeds produced different results");
        std::cout << "HandEngineStressTests PASSED: " << kHands
                  << " hands x 2, reproducible fingerprint=" << a << '\n';
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "HandEngineStressTests FAILED: " << ex.what() << '\n';
        return 1;
    }
}
