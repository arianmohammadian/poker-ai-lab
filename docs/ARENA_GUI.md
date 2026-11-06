# Poker AI Arena — Premium Desktop GUI

**Stack:** C++20 + raylib 6.2 + existing `poker_core` / `GameEngine`.

## Install

1. Extract the archive in the repository root, preserving `apps/`, `assets/`, `docs/`, and `tools/`.
2. Execute `bash install_gui.sh` once. It updates **only** the existing `poker_arena` target in CMake.
3. Run:

```sh
cmake -S . -B build-gui -DPOKER_BUILD_GUI=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-gui --target poker_arena -j 4
./build-gui/poker_arena
```

If raylib dependencies are missing in Ubuntu, install the X11/OpenGL development libraries as previously required.

### Manual CMake configuration (if your CMake layout differs)

Inside your **existing** `if(POKER_BUILD_GUI)` block, retain the raylib `FetchContent` setup and use:

```cmake
add_executable(poker_arena apps/poker_arena.cc apps/arena/arena_view.cc)
target_link_libraries(poker_arena PRIVATE poker_core raylib)
target_compile_definitions(poker_arena PRIVATE
    POKER_ARENA_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/arena"
)
```

Do **not** create a second `poker_arena` target. Do not add raylib to `poker_core`.

## Controls

| Command | Keyboard | Mouse |
|---|---|---|
| Execute one agent action or deal next hand | N | NEXT |
| Start / pause auto play | Space | AUTO PLAY |
| Finish current hand, go to next one | M | NEXT HAND |
| Show / hide both players' cards | H | HIDE / SHOW CARDS |
| Restart deterministically | R | RESTART |
| Playback faster | + | + |
| Playback slower | - | - |
| Quit | Escape | Window close |

The existing fallback `GetCharPressed()` path is kept for WSL/X11 input reliability.

## Architecture

- `apps/poker_arena.cc`: owns one `GameEngine`, two independent `RandomAgent` instances, three RNGs, playback and history. Every state-changing operation occurs here.
- `apps/arena/arena_view.h`: data-only `ArenaSnapshot`, presentation command enum, read-only renderer interface.
- `apps/arena/arena_view.cc`: UI drawing, coordinate scaling, texture loading and hit testing. No poker rules.
- `assets/arena/`: original artwork generated for Poker AI Lab. No third-party card decks, fonts, or trademarks.
- `tools/generate_arena_assets.py`: optional deterministic source generator for the PNG assets (requires Pillow + NumPy only to *regenerate*, not to run the game).

The GUI does not call `HandEngine::applyAction` directly: actions flow through `GameEngine::applyAction`. `ArenaSnapshot` mirrors current actual state and cannot alter gameplay.

## Presentation guarantees

- Fully resizable window with fixed 1600×900 virtual layout and letterboxing.
- Cards rendered from a cached 52-card atlas; no expensive per-frame procedural image generation.
- GPU textures loaded once at startup and unloaded before closing the window.
- Subtle community-card reveal transitions and highlighted actor.
- Auto-play frame-rate independent through `GetFrameTime()`.
- Reproducible replay with seeds `42`, `1234`, `5678`.
- 30 FPS target by default to remain compatible with WSLg/remote OpenGL; this is a performance baseline, not a benchmark.

## Fonts and licenses

The app loads the user's existing system fonts (Inter, Lato, DejaVu Sans, Segoe UI, Arial) with a raylib fallback. **No font files are shipped.** The PNG assets are original, procedurally authored specifically for Poker AI Lab; no licensed third-party playing-card artwork or brand assets are copied.

## Known boundaries

This is a finished *desktop spectator visual module*, not a finished commercial online poker client. There is no Human vs AI, matchmaking, betting UI, or database, and stats beyond stack snapshots are not presented as win-rate estimators. These will be developed independently as the project progresses.

If gameplay is unexpectedly slow, check raylib's graphics backend/WSL configuration and the on-screen FPS. Rendering and game simulation are in the same application thread; extremely expensive future agents should run outside the UI update loop.
