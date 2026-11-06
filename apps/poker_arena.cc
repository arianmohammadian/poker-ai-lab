/**
 * @file poker_arena.cc
 * @brief Poker AI Lab interactive desktop AI-vs-AI spectator.
 * @author Arian Mohammadian
 *
 * Dependencies: poker_core + raylib. GUI input triggers only game commands;
 * rendering is isolated in apps/arena/arena_view.{h,cc}.
 */
#include "arena/arena_view.h"

#include "poker/action.h"
#include "poker/card.h"
#include "poker/game_engine.h"
#include "poker/hand_phase.h"
#include "poker/random_agent.h"
#include "poker/street.h"

#include "raylib.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <exception>
#include <optional>
#include <random>
#include <string>
#include <utility>

namespace {

constexpr std::uint32_t kDeckSeed = 42;
constexpr std::uint32_t kAgent0Seed = 1234;
constexpr std::uint32_t kAgent1Seed = 5678;
constexpr std::int64_t kInitialStack = 1000;
constexpr std::size_t kMaxHistory = 40;

std::string fmt(std::int64_t n) {
    std::string str=std::to_string(n);
    const std::size_t first=(n<0?1:0);
    for(std::size_t p=str.size();p>first+3;p-=3) str.insert(p-3,",");
    return str;
}

std::string streetName(poker::Street s) {
    switch (s) {
        case poker::Street::Preflop: return "PREFLOP";
        case poker::Street::Flop: return "FLOP";
        case poker::Street::Turn: return "TURN";
        case poker::Street::River: return "RIVER";
    }
    return "UNKNOWN";
}

struct Playback {
    bool automatic{false};
    bool reveal{true};
    float delaySeconds{0.85f};
    float elapsed{0.0f};
    float animationAge{5.0f};
    int fromBoard{0};
    std::uint64_t steps{};
    std::string lastEvent{"BLINDS POSTED - READY"};
    std::string error;
    std::deque<arena::ActionRecord> history;
};

void remember(Playback& p,std::string action,std::string context,int kind) {
    p.history.push_front({std::move(action),std::move(context),kind});
    if(p.history.size()>kMaxHistory) p.history.pop_back();
}

std::string actionName(std::uint8_t p,const poker::Action& a) {
    const std::string who="PLAYER "+std::to_string(p)+" ";
    switch(a.type) {
        case poker::ActionType::Fold: return who+"FOLDS";
        case poker::ActionType::Check: return who+"CHECKS";
        case poker::ActionType::Call: return who+"CALLS";
        case poker::ActionType::Bet: return who+"BETS TO "+fmt(a.amount);
        case poker::ActionType::Raise: return who+"RAISES TO "+fmt(a.amount);
    }
    return who+"ACTS";
}

std::string resultText(const poker::HandState& st) {
    if(st.player(0).hasFolded()) return "PLAYER 1 WINS BY FOLD";
    if(st.player(1).hasFolded()) return "PLAYER 0 WINS BY FOLD";
    const std::int64_t a=st.payout(0), b=st.payout(1);
    if(a>b) return "PLAYER 0 WINS THE HAND";
    if(b>a) return "PLAYER 1 WINS THE HAND";
    return "SPLIT POT";
}

arena::VisualCard visualCard(const poker::Card& c) {
    return {static_cast<int>(c.rank()),static_cast<int>(c.suit())};
}

class Session {
public:
    Session() : deckRng_(kDeckSeed), a0Rng_(kAgent0Seed),
                a1Rng_(kAgent1Seed), a0_(a0Rng_), a1_(a1Rng_) {
        reset();
    }

    void reset() {
        deckRng_.seed(kDeckSeed);
        a0Rng_.seed(kAgent0Seed);
        a1Rng_.seed(kAgent1Seed);
        game_.emplace(std::array<std::int64_t,2>{kInitialStack,kInitialStack},
                      poker::HandConfig{50,100},0,deckRng_);
        const bool show=playback_.reveal;
        playback_=Playback{};
        playback_.reveal=show;
        remember(playback_,"HAND #1 DEALT","BLINDS 50 / 100",0);
    }

    void step() {
        if(!game_.has_value()) return;
        try {
            const auto& state=game_->currentHand().state();
            const int oldBoard=static_cast<int>(state.board().size());
            if(state.phase()==poker::HandPhase::Finished) {
                if(game_->isFinished()) {
                    playback_.automatic=false;
                    playback_.lastEvent="MATCH COMPLETE - RESTART TO PLAY AGAIN";
                    return;
                }
                game_->nextHand();
                playback_.lastEvent="HAND #"+std::to_string(game_->handNumber())+" DEALT";
                remember(playback_,playback_.lastEvent,"NEW HAND / BLINDS POSTED",0);
            } else {
                const auto player=state.actingPlayer();
                if(!player) {
                    playback_.automatic=false;
                    playback_.error="Active hand without acting player";
                    return;
                }
                const auto observation=game_->currentHand().observe(*player);
                const poker::Action action=(*player==0)
                    ? a0_.chooseAction(observation) : a1_.chooseAction(observation);
                const std::string name=actionName(*player,action);
                const std::string context=streetName(state.street())+
                    "   /   HAND #"+std::to_string(game_->handNumber());
                game_->applyAction(*player,action);
                playback_.lastEvent=name;
                int kind=(action.type==poker::ActionType::Bet ||
                          action.type==poker::ActionType::Raise)?2:1;
                remember(playback_,name,context,kind);
            }
            ++playback_.steps;
            const auto& newState=game_->currentHand().state();
            const int newBoard=static_cast<int>(newState.board().size());
            playback_.animationAge=0.0f;
            playback_.fromBoard=(newBoard>oldBoard)?oldBoard:5;
            if(newState.phase()==poker::HandPhase::Finished) {
                playback_.lastEvent=resultText(newState);
                remember(playback_,playback_.lastEvent,
                         "HAND #"+std::to_string(game_->handNumber())+" COMPLETE",3);
                if(game_->isFinished()) playback_.automatic=false;
            }
            playback_.error.clear();
        } catch(const std::exception& e) {
            playback_.automatic=false;
            playback_.error=e.what();
            std::fprintf(stderr,"Poker Arena: %s\n",e.what());
        }
    }

    void nextHand() {
        if(game_->isFinished()) {playback_.automatic=false; return;}
        // Fast-forward with a safety cap to avoid freezing the GUI if a future
        // agent unexpectedly produces an endless legal-action sequence.
        const auto target=game_->handNumber()+1;
        for(int i=0;i<512 && game_->handNumber()<target;++i) {
            step();
            if(!playback_.error.empty()) return;
        }
        if(game_->handNumber()<target) {
            playback_.error="Hand skip exceeded safety limit";
            playback_.automatic=false;
        }
    }

    void command(arena::Command c) {
        switch(c) {
            case arena::Command::Next:
                playback_.automatic=false;
                playback_.elapsed=0.0f;
                step(); break;
            case arena::Command::Auto:
                playback_.automatic=!playback_.automatic;
                playback_.elapsed=0.0f; break;
            case arena::Command::NextHand:
                playback_.automatic=false;
                playback_.elapsed=0.0f;
                nextHand(); break;
            case arena::Command::Cards:
                playback_.reveal=!playback_.reveal; break;
            case arena::Command::Reset:
                reset(); break;
            case arena::Command::Slower:
                playback_.delaySeconds=std::min(3.0f,playback_.delaySeconds+.15f);
                playback_.elapsed=0.0f; break;
            case arena::Command::Faster:
                playback_.delaySeconds=std::max(.15f,playback_.delaySeconds-.15f);
                playback_.elapsed=0.0f; break;
        }
    }

    void update(float dt) {
        playback_.animationAge+=dt;
        if(!playback_.automatic) return;
        playback_.elapsed+=dt;
        if(playback_.elapsed>=playback_.delaySeconds) {
            playback_.elapsed=0.0f;
            step();
        }
    }

    [[nodiscard]] arena::ArenaSnapshot snapshot() const {
        arena::ArenaSnapshot s;
        const auto& st=game_->currentHand().state();
        for (int p=0;p<2;++p) {
            auto& v=s.players[p];
            const auto& player=st.player(static_cast<std::uint8_t>(p));
            v.name="PLAYER "+std::to_string(p);
            v.agent="RANDOM AGENT  /  SEED " +
                std::to_string(p==0?kAgent0Seed:kAgent1Seed);
            v.stack=player.stack();
            v.committed=player.committedStreet();
            v.dealer=(st.button()==p);
            v.acting=(st.actingPlayer().has_value() && *st.actingPlayer()==p);
            v.folded=player.hasFolded();
            v.allIn=player.isAllIn();
            for(int i=0;i<2;++i) v.hole[i]=visualCard(player.holeCards()[i]);
        }
        for(std::size_t i=0;i<st.board().size() && i<s.board.size();++i) {
            s.board[i]=visualCard(st.board()[i]);
        }
        s.pot=st.pot();
        s.street=streetName(st.street());
        s.handNumber=game_->handNumber();
        s.steps=playback_.steps;
        s.handFinished=st.phase()==poker::HandPhase::Finished;
        s.gameFinished=game_->isFinished();
        if(s.handFinished) s.result=resultText(st);
        s.event=playback_.lastEvent;
        s.error=playback_.error;
        s.history=playback_.history;
        s.automatic=playback_.automatic;
        s.showHoleCards=playback_.reveal;
        s.secondsPerStep=playback_.delaySeconds;
        s.fps=GetFPS();
        s.newBoardFrom=playback_.fromBoard;
        s.animationAge=playback_.animationAge;
        return s;
    }

private:
    std::mt19937 deckRng_;
    std::mt19937 a0Rng_;
    std::mt19937 a1Rng_;
    poker::RandomAgent a0_;
    poker::RandomAgent a1_;
    std::optional<poker::GameEngine> game_;
    Playback playback_;
};

} // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1440,810,"Poker AI Lab | Arena");
    SetWindowMinSize(1040,585);
    SetTargetFPS(30); // WSL/GLFW-friendly baseline; no forced MSAA.

    arena::ArenaView view;
    if(!view.load()) {
        std::fprintf(stderr,"Cannot load Poker Arena textures. Check assets/arena and the CMake asset path.\n");
        CloseWindow();
        return 1;
    }
    Session session;
    while(!WindowShouldClose()) {
        bool next=IsKeyPressed(KEY_N);
        bool autoPlay=IsKeyPressed(KEY_SPACE);
        bool nextHand=IsKeyPressed(KEY_M);
        bool cards=IsKeyPressed(KEY_H);
        bool reset=IsKeyPressed(KEY_R);
        bool faster=IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD);
        bool slower=IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT);
        // WSL/X11 fallback: preserve both event APIs (do not toggle twice).
        for(int ch=GetCharPressed();ch!=0;ch=GetCharPressed()) {
            if(ch=='n'||ch=='N') next=true;
            if(ch=='m'||ch=='M') nextHand=true;
            if(ch=='h'||ch=='H') cards=true;
            if(ch=='r'||ch=='R') reset=true;
            if(ch==' ') autoPlay=true;
            if(ch=='+') faster=true;
            if(ch=='-') slower=true;
        }
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if(const auto cmd=view.hitTest(view.toCanvas(GetMousePosition()))) {
                session.command(*cmd);
            }
        }
        // Multiple different shortcuts in same frame are deterministic.
        if(reset) session.command(arena::Command::Reset);
        else {
            if(faster&&!slower) session.command(arena::Command::Faster);
            if(slower&&!faster) session.command(arena::Command::Slower);
            if(cards) session.command(arena::Command::Cards);
            if(autoPlay) session.command(arena::Command::Auto);
            if(nextHand) session.command(arena::Command::NextHand);
            else if(next) session.command(arena::Command::Next);
        }
        session.update(std::min(GetFrameTime(),.15f));
        view.draw(session.snapshot());
    }
    view.unload();
    CloseWindow();
    return 0;
}
