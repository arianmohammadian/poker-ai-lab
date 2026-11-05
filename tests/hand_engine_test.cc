/**
 * @file hand_engine_test.cc
 * @brief Integration tests for HandEngine preflop, blinds and all-in resolution.
 */
#include "poker/hand_engine.h"
#include "poker/action.h"
#include "poker/hand_phase.h"

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>

namespace {
using poker::Action;
using poker::ActionType;
using poker::HandConfig;
using poker::HandEngine;
using poker::HandPhase;

int assertions = 0;
void check(bool ok, const std::string& message) {
    ++assertions;
    if (!ok) throw std::runtime_error("TEST FAILED: " + message);
}

template <typename Exception, typename F>
void expectThrows(F&& f, const std::string& message) {
    bool caught = false;
    try { f(); } catch (const Exception&) { caught = true; }
    check(caught, message);
}

std::int64_t chips(const HandEngine& e) {
    const auto& s = e.state();
    return s.player(0).stack() + s.player(1).stack() + s.pot();
}

void finished(const HandEngine& e, std::int64_t total,
              std::int64_t payout, const std::string& label) {
    const auto& s = e.state();
    check(s.phase() == HandPhase::Finished, label + ": wrong phase");
    check(!s.actingPlayer().has_value(), label + ": acting player not cleared");
    check(s.actionsToClose() == 0, label + ": actions remaining");
    check(s.pot() == 0, label + ": unsettled pot");
    check(s.payout(0) + s.payout(1) == payout, label + ": wrong payouts");
    check(chips(e) == total, label + ": chips not conserved");
}

void normalPreflop(std::uint8_t button) {
    std::mt19937 rng{42};
    HandEngine e{{1000, 1000}, HandConfig{50, 100}, button, rng};
    const auto& s = e.state();
    check(s.phase() == HandPhase::Betting, "Normal: phase");
    check(s.street() == poker::Street::Preflop, "Normal: street");
    check(s.board().empty(), "Normal: board");
    check(s.button() == button, "Normal: button");
    check(s.actingPlayer().has_value() && *s.actingPlayer() == button,
          "Normal: SB/BTN must act first");
    check(s.actionsToClose() == 2, "Normal: actionsToClose");
    check(s.lastFullRaise() == 100, "Normal: lastFullRaise");
    check(s.player(button).committedStreet() == 50, "Normal: SB blind");
    check(s.player(1 - button).committedStreet() == 100, "Normal: BB blind");
    check(s.player(button).stack() == 950, "Normal: SB stack");
    check(s.player(1 - button).stack() == 900, "Normal: BB stack");
    check(s.pot() == 150, "Normal: pot");
    check(s.payout(0) == 0 && s.payout(1) == 0, "Normal: payouts");
    check(chips(e) == 2000, "Normal: chip conservation");
}

void sbAllInAutomatically() {
    std::mt19937 rng{11};
    HandEngine e{{30, 1000}, HandConfig{50, 100}, 0, rng};
    const auto& s = e.state();
    finished(e, 1030, 60, "SB all-in");
    check(s.board().size() == 5, "SB all-in: board");
    check(s.player(0).committedTotal() == 30, "SB all-in: SB committed");
    check(s.player(1).committedTotal() == 30, "SB all-in: BB refund");
}

void bbAllInAutomatically() {
    std::mt19937 rng{12};
    HandEngine e{{1000, 30}, HandConfig{50, 100}, 0, rng};
    const auto& s = e.state();
    finished(e, 1030, 60, "BB short all-in");
    check(s.board().size() == 5, "BB short all-in: board");
    check(s.player(0).committedTotal() == 30, "BB short all-in: SB refund");
    check(s.player(1).committedTotal() == 30, "BB short all-in: BB committed");
}

void bothAllInAutomatically() {
    std::mt19937 rng{13};
    HandEngine e{{30, 70}, HandConfig{50, 100}, 0, rng};
    const auto& s = e.state();
    finished(e, 100, 60, "Both all-in");
    check(s.board().size() == 5, "Both all-in: board");
    check(s.player(0).committedTotal() == 30, "Both all-in: SB committed");
    check(s.player(1).committedTotal() == 30, "Both all-in: BB refund");
}

void bbAllInWithDecision(std::uint8_t button) {
    std::mt19937 rng{14};
    std::array<std::int64_t, 2> stacks{1000, 1000};
    stacks[1 - button] = 70;
    HandEngine e{stacks, HandConfig{50, 100}, button, rng};
    const auto& s = e.state();
    check(s.phase() == HandPhase::Betting, "BB all-in: Betting required");
    check(s.actingPlayer().has_value() && *s.actingPlayer() == button,
          "BB all-in: SB must decide");
    check(s.actionsToClose() == 1, "BB all-in: one action pending");
    check(s.board().empty(), "BB all-in: no board yet");
    check(s.pot() == 120, "BB all-in: pot before decision");
    check(s.player(button).committedTotal() == 50, "BB all-in: SB blind");
    check(s.player(1 - button).committedTotal() == 70, "BB all-in: BB blind");
    check(chips(e) == 1070, "BB all-in: chip conservation");
}

void foldAgainstAllIn(std::uint8_t button) {
    std::mt19937 rng{15};
    std::array<std::int64_t, 2> stacks{1000, 1000};
    stacks[1 - button] = 70;
    HandEngine e{stacks, HandConfig{50, 100}, button, rng};
    e.applyAction(button, Action{ActionType::Fold, 0});
    const auto& s = e.state();
    finished(e, 1070, 120, "Fold");
    check(s.player(button).hasFolded(), "Fold: SB folded");
    check(!s.player(1 - button).hasFolded(), "Fold: BB not folded");
    check(s.player(button).stack() == 950, "Fold: SB final stack");
    check(s.player(1 - button).stack() == 120, "Fold: BB final stack");
    check(s.board().empty(), "Fold: no runout");
    check(s.payout(button) == 0, "Fold: SB payout");
    check(s.payout(1 - button) == 120, "Fold: BB payout");
    expectThrows<std::logic_error>([&] {
        e.applyAction(button, Action{ActionType::Fold, 0});
    }, "Fold: cannot act after settlement");
}

void callAgainstAllIn(std::uint8_t button) {
    std::mt19937 rng{16};
    std::array<std::int64_t, 2> stacks{1000, 1000};
    stacks[1 - button] = 70;
    HandEngine e{stacks, HandConfig{50, 100}, button, rng};
    e.applyAction(button, Action{ActionType::Call, 0});
    const auto& s = e.state();
    finished(e, 1070, 140, "Call");
    check(s.board().size() == 5, "Call: full board");
    check(s.player(button).committedTotal() == 70, "Call: SB commitment");
    check(s.player(1 - button).committedTotal() == 70, "Call: BB commitment");
    check(!s.player(button).hasFolded(), "Call: SB must not fold");
    check(!s.player(1 - button).hasFolded(), "Call: BB must not fold");
    expectThrows<std::logic_error>([&] {
        e.applyAction(button, Action{ActionType::Call, 0});
    }, "Call: cannot act after settlement");
}

void illegalActionsPreserveState() {
    std::mt19937 rng{17};
    HandEngine e{{1000, 70}, HandConfig{50, 100}, 0, rng};
    const auto pot = e.state().pot();
    const auto stack0 = e.state().player(0).stack();
    const auto stack1 = e.state().player(1).stack();
    expectThrows<std::invalid_argument>([&] {
        e.applyAction(1, Action{ActionType::Call, 0});
    }, "Illegal: wrong turn");
    expectThrows<std::invalid_argument>([&] {
        e.applyAction(0, Action{ActionType::Raise, 0});
    }, "Illegal: unsupported Raise");
    expectThrows<std::invalid_argument>([&] {
        e.applyAction(0, Action{ActionType::Call, 10});
    }, "Illegal: Call with amount");
    expectThrows<std::invalid_argument>([&] {
        e.applyAction(0, Action{ActionType::Check, 0});
    }, "Illegal: unsupported Check");
    check(e.state().pot() == pot, "Illegal: pot mutated");
    check(e.state().player(0).stack() == stack0, "Illegal: SB stack mutated");
    check(e.state().player(1).stack() == stack1, "Illegal: BB stack mutated");
    check(e.state().actingPlayer().has_value() && *e.state().actingPlayer() == 0,
          "Illegal: acting player mutated");
    check(e.state().actionsToClose() == 1, "Illegal: counter mutated");
}

void invalidConstructorInputs() {
    const HandConfig blinds{50, 100};
    expectThrows<std::invalid_argument>([&] {
        std::mt19937 rng{1};
        HandEngine e{{1000, 1000}, blinds, 2, rng};
    }, "Invalid constructor: button");
    expectThrows<std::invalid_argument>([&] {
        std::mt19937 rng{1};
        HandEngine e{{0, 1000}, blinds, 0, rng};
    }, "Invalid constructor: zero stack");
    expectThrows<std::invalid_argument>([&] {
        std::mt19937 rng{1};
        HandEngine e{{1000, 1000}, HandConfig{100, 100}, 0, rng};
    }, "Invalid constructor: equal blinds");
    expectThrows<std::invalid_argument>([&] {
        std::mt19937 rng{1};
        HandEngine e{{1000, 1000}, HandConfig{0, 100}, 0, rng};
    }, "Invalid constructor: zero SB");
    expectThrows<std::invalid_argument>([&] {
        std::mt19937 rng{1};
        HandEngine e{{std::numeric_limits<std::int64_t>::max(), 1}, blinds, 0, rng};
    }, "Invalid constructor: stack sum overflow");
}
// ADD these functions inside the anonymous namespace in hand_engine_test.cc,
// BEFORE the '} // namespace'. Then call them from main().

void fullHandChecksToShowdown(std::uint8_t button) {
    std::mt19937 rng{123};
    HandEngine e{{1000, 1000}, HandConfig{50, 100}, button, rng};
    const std::uint8_t bb = 1 - button;
    e.applyAction(button, {ActionType::Call, 0});
    e.applyAction(bb, {ActionType::Check, 0});
    check(e.state().street() == poker::Street::Flop, "Flop not reached");
    check(e.state().board().size() == 3, "Flop must have 3 cards");
    check(e.state().actingPlayer() == bb, "BB first on flop");
    check(e.state().pot() == 200, "Flop pot must be 200");
    for (auto street : {poker::Street::Flop, poker::Street::Turn,
                        poker::Street::River}) {
        check(e.state().street() == street, "Unexpected betting street");
        e.applyAction(bb, {ActionType::Check, 0});
        e.applyAction(button, {ActionType::Check, 0});
    }
    finished(e, 2000, 200, "Check-down to river");
    check(e.state().board().size() == 5, "River must contain five cards");
}

void betCallAndRaiseSequence() {
    std::mt19937 rng{124};
    HandEngine e{{1000, 1000}, HandConfig{50, 100}, 0, rng};
    e.applyAction(0, {ActionType::Raise, 300}); // SB raises TO 300
    check(e.state().lastFullRaise() == 200, "Raise increment must be 200");
    check(e.state().pot() == 400, "Pot after raise must be 400");
    e.applyAction(1, {ActionType::Call, 0});
    check(e.state().street() == poker::Street::Flop, "Raise/call to flop");
    check(e.state().pot() == 600, "Pot must be 600 after raise/call");
    e.applyAction(1, {ActionType::Bet, 100});
    e.applyAction(0, {ActionType::Raise, 300}); // 200 increment
    check(e.state().lastFullRaise() == 200, "Raise should update minimum increment");
    e.applyAction(1, {ActionType::Call, 0});
    check(e.state().street() == poker::Street::Turn, "Bet/raise/call to turn");
    check(e.state().pot() == 1200, "Pot must be 1200 on turn");
    e.applyAction(1, {ActionType::Check, 0});
    e.applyAction(0, {ActionType::Check, 0});
    e.applyAction(1, {ActionType::Check, 0});
    e.applyAction(0, {ActionType::Check, 0});
    finished(e, 2000, 1200, "Raise/call hand");
}

void bbRaisesAfterSbLimp() {
    std::mt19937 rng{125};
    HandEngine e{{1000, 1000}, HandConfig{50, 100}, 0, rng};
    e.applyAction(0, {ActionType::Call, 0});
    e.applyAction(1, {ActionType::Raise, 300}); // Legal BB option
    check(e.state().actingPlayer() == 0, "SB must respond to BB raise");
    check(e.state().actionsToClose() == 1, "One response outstanding");
    e.applyAction(0, {ActionType::Fold, 0});
    finished(e, 2000, 400, "BB raise after limp + fold");
}

void partialAllInCallAndRefund() {
    std::mt19937 rng{126};
    HandEngine e{{350, 1000}, HandConfig{50, 100}, 0, rng};
    e.applyAction(0, {ActionType::Raise, 350}); // SB all-in to 350
    e.applyAction(1, {ActionType::Call, 0});
    finished(e, 1350, 700, "All-in raise called");
    check(e.state().board().size() == 5, "All-in runout must finish board");
}

void shortCallBelowOwedAmount() {
    std::mt19937 rng{127};
    HandEngine e{{200, 1000}, HandConfig{50, 100}, 0, rng};
    // BB can only receive an all-in raise if SB has more than 100.
    // SB pushes 200; BB calls normally: tests all-in resolution.
    e.applyAction(0, {ActionType::Raise, 200});
    e.applyAction(1, {ActionType::Call, 0});
    finished(e, 1200, 400, "Short stack all-in raise");
}

void smallAllInOpeningBet() {
    std::mt19937 rng{128};
    HandEngine e{{1000, 150}, HandConfig{50, 100}, 0, rng};
    e.applyAction(0, {ActionType::Call, 0});
    e.applyAction(1, {ActionType::Check, 0});
    // BB has 50 left, can bet this amount all-in on flop (<BB).
    e.applyAction(1, {ActionType::Bet, 50});
    check(e.state().actingPlayer() == 0, "SB must respond to small all-in bet");
    e.applyAction(0, {ActionType::Call, 0});
    finished(e, 1150, 300, "Small all-in opening bet");
}

void rejectedRaiseMustNotChangeChips() {
    std::mt19937 rng{129};
    HandEngine e{{1000, 1000}, HandConfig{50, 100}, 0, rng};
    const auto pot = e.state().pot();
    const auto stack = e.state().player(0).stack();
    expectThrows<std::invalid_argument>([&] {
        e.applyAction(0, {ActionType::Raise, 150}); // min raise to 200
    }, "Illegal below-minimum non-all-in raise must be rejected");
    expectThrows<std::invalid_argument>([&] {
        e.applyAction(0, {ActionType::Bet, 100}); // already opened by BB
    }, "Bet cannot be used to raise a blind");
    check(e.state().pot() == pot, "Rejected raise must not change pot");
    check(e.state().player(0).stack() == stack, "Rejected raise must not change stack");
    check(e.state().actingPlayer() == 0, "Rejected raise must preserve turn");
}


void partialCallAllInAfterLargeRaise() {
    std::mt19937 rng{130};
    HandEngine e{{1000, 200}, HandConfig{50, 100}, 0, rng};
    e.applyAction(0, {ActionType::Raise, 500});
    // BB has only 100 left and cannot cover the additional 400.
    e.applyAction(1, {ActionType::Call, 0});
    finished(e, 1200, 400, "Partial all-in call");
    check(e.state().player(0).committedTotal() == 200,
          "Excess 300 chips must be refunded to raiser");
    check(e.state().player(1).committedTotal() == 200,
          "Short caller must put in their entire stack");
    check(e.state().board().size() == 5, "Partial all-in must run out board");
}

void incompleteAllInRaiseIsLegal() {
    std::mt19937 rng{131};
    HandEngine e{{175, 1000}, HandConfig{50, 100}, 0, rng};
    // SB raises all-in to 175. The increment is only 75 (< BB=100).
    e.applyAction(0, {ActionType::Raise, 175});
    check(e.state().lastFullRaise() == 100,
          "Incomplete all-in must not update last full raise size");
    e.applyAction(1, {ActionType::Call, 0});
    finished(e, 1175, 350, "Incomplete all-in raise");
}

void checkThenBetAndFold() {
    std::mt19937 rng{132};
    HandEngine e{{1000, 1000}, HandConfig{50, 100}, 0, rng};
    e.applyAction(0, {ActionType::Call, 0});
    e.applyAction(1, {ActionType::Check, 0});
    e.applyAction(1, {ActionType::Check, 0});
    e.applyAction(0, {ActionType::Bet, 150});
    check(e.state().actingPlayer() == 1, "Check/bet must return turn to BB");
    e.applyAction(1, {ActionType::Fold, 0});
    finished(e, 2000, 350, "Check/bet/fold");
}

} // namespace

int main() {
    try {
        normalPreflop(0);
        normalPreflop(1);
        sbAllInAutomatically();
        bbAllInAutomatically();
        bothAllInAutomatically();
        bbAllInWithDecision(0);
        bbAllInWithDecision(1);
        foldAgainstAllIn(0);
        foldAgainstAllIn(1);
        callAgainstAllIn(0);
        callAgainstAllIn(1);
        illegalActionsPreserveState();
        invalidConstructorInputs();
        fullHandChecksToShowdown(0);
        fullHandChecksToShowdown(1);
        betCallAndRaiseSequence();
        bbRaisesAfterSbLimp();
        partialAllInCallAndRefund();
        shortCallBelowOwedAmount();
        smallAllInOpeningBet();
        rejectedRaiseMustNotChangeChips();
        partialCallAllInAfterLargeRaise();
        incompleteAllInRaiseIsLegal();
        checkThenBetAndFold();
        std::cout << "HandEngineTests PASSED (" << assertions << " assertions)\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
}
