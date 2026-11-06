/**
 * @file arena_view.h
 * @brief Presentation-only model and raylib renderer for Poker AI Arena.
 * @author Arian Mohammadian
 */
#pragma once

#include "raylib.h"

#include <array>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace arena {

constexpr int kVirtualWidth = 1600;
constexpr int kVirtualHeight = 900;

struct VisualCard {
    int rank{};   // 2..14
    int suit{};   // 0 Clubs, 1 Diamonds, 2 Hearts, 3 Spades
};

struct VisualPlayer {
    std::string name;
    std::string agent;
    std::int64_t stack{};
    std::int64_t committed{};
    std::array<VisualCard, 2> hole{};
    bool dealer{};
    bool acting{};
    bool folded{};
    bool allIn{};
};

struct ActionRecord {
    std::string action;
    std::string context;
    int kind{};     // 0 neutral, 1 passive, 2 aggressive, 3 result
};

struct ArenaSnapshot {
    std::array<VisualPlayer, 2> players{};
    std::array<std::optional<VisualCard>, 5> board{};
    std::int64_t pot{};
    std::uint64_t handNumber{};
    std::uint64_t steps{};
    std::string street{"PREFLOP"};
    std::string event;
    std::string result;
    std::string error;
    std::deque<ActionRecord> history;
    bool handFinished{};
    bool gameFinished{};
    bool automatic{};
    bool showHoleCards{true};
    float secondsPerStep{0.8f};
    int fps{};
    int newBoardFrom{0};   // Animation: first newly-dealt community-card index
    float animationAge{5.0f};
};

enum class Command { Next, Auto, NextHand, Cards, Reset, Slower, Faster };

class ArenaView {
public:
    ArenaView() = default;
    ArenaView(const ArenaView&) = delete;
    ArenaView& operator=(const ArenaView&) = delete;

    [[nodiscard]] bool load();
    void unload();
    void draw(const ArenaSnapshot& scene) const;
    [[nodiscard]] Vector2 toCanvas(Vector2 screenPoint) const;
    [[nodiscard]] std::optional<Command> hitTest(Vector2 canvasPoint) const;

private:
    struct Fonts {
        Font regular{};
        Font semibold{};
        Font display{};
        bool ownsRegular{};
        bool ownsSemibold{};
        bool ownsDisplay{};
    } fonts_{};

    Texture2D background_{};
    Texture2D table_{};
    Texture2D cards_{};
    Texture2D back_{};

    void drawHeader(const ArenaSnapshot& scene) const;
    void drawTable(const ArenaSnapshot& scene) const;
    void drawSeats(const ArenaSnapshot& scene) const;
    void drawSidebar(const ArenaSnapshot& scene) const;
    void drawControls(const ArenaSnapshot& scene) const;
};

} // namespace arena
