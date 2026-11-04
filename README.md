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

**Status: Active Development — v0.1 Poker Core**

Development is currently focused on building a reliable and independently tested foundation for Texas Hold'em.

### Implemented

- [x] Strongly typed card representation (`Rank`, `Suit`, `Card`)
- [x] Standard 52-card deck
- [x] Hand strength representation and comparison (`HandRank`)
- [x] Five-card poker hand evaluator
- [x] Hand classification, kicker comparison, and tie-breaking
- [x] Ace-low straight handling
- [x] Input validation and duplicate-card detection
- [x] Automated component tests
- [x] Exhaustive five-card hand category verification
- [x] CMake build system with C++20
- [x] Continuous integration using GitHub Actions

### In Progress / Next Steps

- [ ] Seven-card hand evaluation (best five out of seven)
- [ ] Additional independent correctness validation
- [ ] Performance benchmarks
- [ ] Sanitizer-based validation
- [ ] Minimal command-line demonstration

### Exhaustive Hand Evaluation Testing

The five-card evaluator includes an exhaustive test covering all:

**2,598,960 possible five-card combinations**

The test verifies the distribution of standard poker hand categories against their mathematically known frequencies.

| Hand Category   |  Combinations |
| --------------- | ------------: |
| Straight Flush  |            40 |
| Four of a Kind  |           624 |
| Full House      |         3,744 |
| Flush           |         5,108 |
| Straight        |        10,200 |
| Three of a Kind |        54,912 |
| Two Pair        |       123,552 |
| One Pair        |     1,098,240 |
| High Card       |     1,302,540 |
| **Total**       | **2,598,960** |

Additional tests cover hand comparisons, tie-breaking, invalid inputs, and important edge cases.

This validation provides a foundation for future seven-card evaluation, equity calculations, and poker simulations.

---

## Project Components

| Component         | Purpose                                             | Status         |
| ----------------- | --------------------------------------------------- | -------------- |
| Poker Core        | Cards, deck management, hand evaluation             | In development |
| Equity Calculator | Exact enumeration and Monte Carlo simulation        | Planned        |
| Game Engine       | Heads-Up NLHE rules, betting, and state transitions | Planned        |
| AI Agents         | Random, heuristic, and learned strategies           | Planned        |
| Poker Solver      | CFR and imperfect-information game solving          | Planned        |
| Training          | Self-play and reproducible learning experiments     | Planned        |
| Poker Arena       | Human-versus-AI and AI-versus-AI gameplay           | Planned        |
| Analytics         | Benchmarks, statistics, and strategy visualization  | Planned        |

The components describe the long-term architecture. They will be implemented incrementally rather than introduced as empty modules.

## Development Roadmap

Development follows independently testable milestones, prioritizing correctness before optimization.

| Version | Milestone                        | Status      |
| ------- | -------------------------------- | ----------- |
| v0.1    | Poker Core & Hand Evaluator      | In progress |
| v0.2    | Equity Calculator                | Planned     |
| v0.3    | Heads-Up NLHE Game Engine        | Planned     |
| v0.4    | First AI Arena (CLI)             | Planned     |
| v0.5    | CFR Research Lab                 | Planned     |
| v0.6    | Self-Play Training               | Planned     |
| v0.7+   | Visual Arena & Advanced Research | Planned     |

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

The project uses automated C++ test executables registered with CTest.

Continuous integration runs the build and test suite through GitHub Actions for both Debug and Release configurations.

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

Simplified games such as Kuhn Poker and Leduc Poker may be implemented as isolated research experiments to validate game-theoretic algorithms before applying them to Texas Hold'em.

**Solving full-scale No-Limit Texas Hold'em is not an initial objective.**

The emphasis is on progressively building, understanding, and evaluating increasingly sophisticated poker intelligence.

## Documentation

- [Development Roadmap](docs/ROADMAP.md) — Milestones, deliverables, and acceptance criteria.
- [Software Architecture](docs/ARCHITECTURE.md) — Architectural principles, component responsibilities, and design decisions.

Documentation will evolve alongside the implementation, benchmarks, and experimental results.

## License

This project is licensed under the [MIT License](LICENSE).

Copyright © 2026 Arian Mohammadian.
