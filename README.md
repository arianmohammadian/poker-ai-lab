# ♠ Poker AI Lab

### From poker rules to poker intelligence.

**Engineering a Texas Hold'em platform where AI agents can play, learn, compete, and evolve.**

Poker AI Lab is a long-term **C++20 software engineering and artificial intelligence project** focused on Heads-Up No-Limit Texas Hold'em.

The goal is to build a modular platform combining a high-performance poker engine, game-theoretic solvers, self-learning AI agents, and an interactive environment for gameplay and analysis.

Starting from fundamental poker algorithms, the project progressively explores **probability, game theory, reinforcement learning, self-play, and high-performance computing**.

The ultimate challenge: **can we build a poker AI that learns to become a stronger player?**

---

## Vision

Poker AI Lab is built around three main capabilities:

- **Play** — Challenge custom poker agents and observe AI-versus-AI matches.
- **Train** — Develop intelligent agents that improve through self-play and experimentation.
- **Analyze** — Calculate equity, investigate strategies, approximate game-theoretic equilibria, and evaluate agent performance.

Rather than relying on existing poker engines or solvers, the project emphasizes implementing and understanding the underlying algorithms from the ground up.

Correctness, reproducibility, and measurable progress are fundamental to its development.

## Current Progress

**Status: v0.3 — Heads-Up Hand Engine implemented and locally tested; v0.4 in planning**

The Poker Core, Equity Calculator, and Heads-Up hand engine milestones have been implemented and validated with automated tests.

The project provides five-card and seven-card hand evaluation, exact Heads-Up equity computation, reproducible Monte Carlo simulation, an interactive equity CLI, and a reusable engine for executing individual Heads-Up NLHE hands. A playable match CLI and AI agents are not yet included.

### Implemented

**v0.1 — Poker Core & Hand Evaluation**

- [x] Strongly typed card representation (`Rank`, `Suit`, `Card`)
- [x] Standard 52-card deck with deterministic shuffling
- [x] Hand strength representation and comparison (`HandRank`)
- [x] Five-card poker hand evaluator
- [x] Seven-card hand evaluator (best five out of seven)
- [x] Hand classification, kicker comparison, and tie-breaking
- [x] Ace-low straight handling
- [x] Input validation and duplicate-card detection
- [x] Automated component tests
- [x] Exhaustive five-card hand category verification (2,598,960 combinations)
- [x] CMake build system with C++20
- [x] Continuous integration using GitHub Actions

**v0.2 — Equity Calculator**

- [x] Heads-Up equity calculation with two known private hands
- [x] Support for preflop, flop, turn, and river
- [x] Exact enumeration of all valid board completions
- [x] Monte Carlo simulation with configurable sample counts
- [x] Seeded pseudo-random sampling for reproducible experiments
- [x] Shared showdown evaluation using the existing `HandEvaluator`
- [x] Win, loss, tie, and equity statistics
- [x] Interactive CLI supporting both calculation methods
- [x] Input validation and CLI integration tests
- [x] Automated tests covering equity calculation and CLI behavior

**v0.3 — Heads-Up NLHE Hand Engine**

- [x] `PlayerState`, `HandState`, `HandEngine`, and player action representation
- [x] Heads-Up button, blind posting, and preflop action order
- [x] `Fold`, `Check`, `Call`, `Bet`, and `Raise` with validation
- [x] Minimum raises, short all-ins, and unmatched-chip refunds
- [x] Preflop, flop, turn, and river transitions with burn cards
- [x] Automatic runouts, showdown, and pot settlement
- [x] Seeded dealing for repeatable test scenarios
- [x] Integration tests and randomized stress testing (10,000 hands, repeated with the same seed)
- [x] Complete local CTest suite: **15/15 tests passing** (as reported for v0.3)

**Next Steps — v0.4 First AI Arena**

- [ ] Introduce a player-specific observation API that hides opponent hole cards and undealt cards
- [ ] Implement a legal-action interface for agents and human input
- [ ] Introduce a match coordinator for multiple hands, stack carry-over, and button rotation
- [ ] Build a playable Heads-Up command-line application
- [ ] Add baseline agents and reproducible agent-versus-agent matches
- [ ] Confirm the v0.3 commit passes both Debug and Release CI workflows

**Ongoing quality work:** sanitizer runs, independent rule/reference checks where practical, and performance benchmarks.

### Exhaustive Hand Evaluation Testing

The five-card evaluator includes an exhaustive test covering all:

**2,598,960 possible five-card combinations**

The test verifies the distribution of standard poker hand categories against their mathematically known frequencies.

| Hand Category | Combinations |
|---|---:|
| Straight Flush | 40 |
| Four of a Kind | 624 |
| Full House | 3,744 |
| Flush | 5,108 |
| Straight | 10,200 |
| Three of a Kind | 54,912 |
| Two Pair | 123,552 |
| One Pair | 1,098,240 |
| High Card | 1,302,540 |
| **Total** | **2,598,960** |

Additional tests cover hand comparisons, tie-breaking, invalid inputs, and important edge cases.

This validation provides a reliable foundation for the completed seven-card evaluator and equity calculator, as well as future poker simulations, game engines, and AI training experiments.

---

## Equity Calculator

Poker AI Lab includes a Heads-Up Texas Hold'em equity calculator supporting **exact enumeration and Monte Carlo simulation**.

Given two known private hands and an optional community board, the calculator determines each player's equity, accounting for victories, losses, and split pots.

### Calculation Methods

**Exact Enumeration**

Enumerates every possible completion of the community board without duplicate combinations, using recursive backtracking.

| Street | Unknown Board Cards | Combinations |
|---|---:|---:|
| Preflop | 5 | 1,712,304 |
| Flop | 2 | 990 |
| Turn | 1 | 44 |
| River | 0 | 1 |

Each unique board completion is evaluated exactly once using the seven-card hand evaluator.

**Monte Carlo Simulation**

Estimates equity by sampling valid board completions using partial Fisher-Yates shuffling and a seeded `std::mt19937_64` pseudo-random generator.

The number of samples and random seed are configurable. Repeated calculations with identical parameters are reproducible within the same implementation environment.

Each simulation generates a valid completion of the community board without repeated cards. Different simulations may independently sample the same completion.

Both algorithms reuse the existing seven-card `HandEvaluator` through a shared showdown evaluation routine.

### Interactive CLI

The calculator provides an interactive command-line interface:

```bash
./build/poker_equity
```

Example:

```text
================================
         POKER AI LAB
     HU Equity Calculator
================================

Player 1 (e.g. Ah As): Ah As
Player 2 (e.g. Kh Ks): Kh Ks
Board (empty / 3 / 4 / 5 cards):

Calculation mode:
  1. Exact enumeration
  2. Monte Carlo
Choose mode: 1

Calculating exact equity...

--------------------------------
Player 1 equity: 82.64%
Player 2 equity: 17.36%

Player 1 wins: 1410336
Player 2 wins: 292660
Ties:          9308
Scenarios:     1712304
Method:        Exact
--------------------------------
```

The example shows the exact preflop equity of `Ah As` against `Kh Ks`, calculated across all 1,712,304 possible community boards.

**Card notation**

Cards use two-character notation:

- **Ranks:** `2`–`9`, `T`, `J`, `Q`, `K`, `A`
- **Suits:** `c` (clubs), `d` (diamonds), `h` (hearts), `s` (spades)

Leave the board empty for preflop, or provide exactly three, four, or five cards for flop, turn, and river.

For Monte Carlo calculations, select mode `2` and provide a positive sample count and a random seed.

### Equity Definition

For Heads-Up Texas Hold'em, each player's equity includes half the probability of a tied outcome:

\[
Equity(P1) = \frac{Wins(P1) + Ties/2}{Total}
\]

\[
Equity(P2) = \frac{Wins(P2) + Ties/2}{Total}
\]

Exact enumeration produces exact probabilities under uniformly distributed unknown community cards.

Monte Carlo produces statistical estimates that converge toward the exact probabilities as the sample count increases.

### Validation

The Equity Calculator includes automated tests covering:

- Exact enumeration across all four streets
- Hand comparisons and tied outcomes
- Expected numbers of board completions
- Monte Carlo reproducibility with fixed seeds
- Monte Carlo convergence against exact results
- Invalid board sizes and duplicate cards
- CLI input validation and integration

The complete project test suite has **15/15 local CTest tests passing** following v0.3 integration.

---

## Project Components

| Component | Purpose | Status |
|---|---|---|
| Poker Core | Cards, deck management, five-card and seven-card hand evaluation | Completed (v0.1) |
| Equity Calculator | Exact enumeration, Monte Carlo simulation, and interactive CLI | Completed (v0.2) |
| Hand Engine | Individual Heads-Up NLHE hands, betting, street transitions, and settlement | Implemented and locally tested (v0.3) |
| Match Coordinator | Multiple hands, button rotation, and match lifecycle | Planned (v0.4) |
| AI Agents | Random, heuristic, and learned strategies | Planned |
| Poker Solver | CFR and imperfect-information game solving | Planned |
| Training | Self-play and reproducible learning experiments | Planned |
| Poker Arena | Human-versus-AI and AI-versus-AI gameplay | Planned |
| Analytics | Benchmarks, statistics, and strategy visualization | Planned |

The components describe the long-term architecture. They will be implemented incrementally rather than introduced as empty modules.

## Development Roadmap

Development follows independently testable milestones, prioritizing correctness before optimization.

| Version | Milestone | Status |
|---|---|---|
| v0.1 | Poker Core & Hand Evaluator | Completed |
| v0.2 | Equity Calculator | Completed |
| v0.3 | Heads-Up NLHE Hand Engine | Implemented; local tests passing |
| v0.4 | First AI Arena (CLI) | Planned |
| v0.5 | CFR Research Lab | Planned |
| v0.6 | Self-Play Training | Planned |
| v0.7+ | Visual Arena & Advanced Research | Planned |

Each milestone has explicit functionality, testing requirements, and acceptance criteria.

See the [Development Roadmap](docs/ROADMAP.md) for details.

## Technology Stack

**Core Development**

- C++20
- C++ Standard Library
- CMake
- CTest

**Development & Quality**

- GCC
- Ubuntu / WSL
- Visual Studio Code
- Git & GitHub
- GitHub Actions (CI)

**Planned Technologies**

- Python for experiments, benchmarking, and data analysis
- React and TypeScript for the future visual arena

The core poker algorithms are implemented in C++ rather than delegated to external poker libraries.

## Building & Testing

### Requirements

- A C++20-compatible compiler (GCC recommended)
- CMake 3.16 or newer
- Git

### Clone

```bash
git clone https://github.com/arianmohammadian/poker-ai-lab.git
cd poker-ai-lab
```

### Build

Configure and compile the project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

For a Debug build, use a separate build directory:

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```

### Run Tests

```bash
ctest --test-dir build --output-on-failure
```

The project uses automated C++ test executables registered with CTest. The v0.3 suite includes integration scenarios and randomized, fixed-seed engine stress tests; these complement rather than replace independent rule validation.

Continuous integration runs the build and test suite through GitHub Actions for both Debug and Release configurations.

### Run the Equity Calculator

After building the project, launch the interactive equity calculator:

```bash
./build/poker_equity
```

The application supports exact enumeration and Monte Carlo simulation for two known Heads-Up Texas Hold'em hands. **This is currently the interactive CLI:** the hand engine is a C++ library and the playable poker CLI is planned for v0.4.

For Monte Carlo calculations, specify a positive sample count and a random seed.

To run the Equity Calculator tests individually:

```bash
ctest --test-dir build -R equity --output-on-failure
```

To run the complete project test suite:

```bash
ctest --test-dir build --output-on-failure
```

## Engineering Principles

**1. Correctness Before Optimization**

Game rules and algorithms must be thoroughly tested before performance improvements are introduced.

**2. Modular Architecture**

Components should have clear responsibilities, explicit dependencies, and reusable interfaces.

**3. Reproducibility**

Simulations and experiments should use controlled randomness, documented configurations, and repeatable evaluation procedures.

**4. Performance Through Measurement**

Optimizations should be justified by benchmarks and profiling rather than assumptions.

**5. Incremental Development**

Each milestone should produce working, verifiable functionality before expanding the project's scope.

**6. Algorithmic Understanding**

Core algorithms are implemented, analyzed, and tested rather than treated as black boxes.

## Research Direction

The long-term research objectives include:

- Exact and approximate poker equity computation
- Counterfactual Regret Minimization (CFR)
- CFR+ and alternative regret-minimization techniques
- Imperfect-information game solving
- Reinforcement learning and self-play
- Exploitative and approximate equilibrium strategies
- Statistical evaluation of agent performance
- Efficient simulation and parallel computation

Research and experimentation will focus exclusively on Texas Hold'em, beginning with Heads-Up play and progressively introducing more sophisticated solving and learning techniques.

**Solving full-scale No-Limit Texas Hold'em is not an initial objective.**

The emphasis is on progressively building, understanding, and evaluating increasingly sophisticated poker intelligence.

## Documentation

- [Development Roadmap](docs/ROADMAP.md) — Milestones, deliverables, and acceptance criteria.
- [Software Architecture](docs/ARCHITECTURE.md) — Architectural principles, component responsibilities, and design decisions.

Documentation will evolve alongside the implementation, benchmarks, and experimental results.

## License

This project is licensed under the [MIT License](LICENSE).

Copyright © 2026 Arian Mohammadian.