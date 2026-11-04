# ♠ Poker AI Lab

### From poker rules to poker intelligence.

**Building a Texas Hold'em engine, game-theoretic solvers, and AI agents that learn through self-play.**

Poker AI Lab is a long-term C++20 software engineering and artificial intelligence project focused on **Heads-Up No-Limit Texas Hold'em**.

The goal is to develop a platform where poker agents can play, learn, compete, and evolve through reproducible experiments.

From hand evaluation and probability calculations to imperfect-information game solving and self-play, the project explores the intersection of **algorithms, probability, game theory, reinforcement learning, and high-performance computing**.

## Vision

Poker AI Lab is built around three main capabilities:

- **Play:** Compete against custom poker agents and observe AI-versus-AI matches.
- **Train:** Develop increasingly sophisticated agents and investigate learning through self-play.
- **Analyze:** Calculate equity, study strategies, explore game-theoretic solutions, and evaluate agent performance.

The ultimate goal is not simply to build a poker game, but to create an environment for engineering, training, and understanding poker intelligence.

## Current Progress

**Status: Active Development — v0.1 Poker Core**

The project has moved beyond initial planning and into implementation.

### Implemented

- Card representation using C++20.
- Five-card poker hand evaluation.
- Hand classification and tie-breaking logic.
- Automated tests for the hand evaluator.
- Continuous integration (CI) for automated validation.
- CMake-based build configuration.

### In Progress

- Completing the Poker Core milestone.
- Extending hand evaluation to seven-card Texas Hold'em hands.
- Expanding correctness tests and performance benchmarks.

## Project Components

| Component | Description |
|---|---|
| Poker Core | Card representation, deck management, and hand evaluation |
| Equity Calculator | Exact equity computation and Monte Carlo simulation |
| Game Engine | Heads-Up No-Limit Texas Hold'em rules and betting |
| AI Agents | Baseline, heuristic, and learned poker agents |
| Poker Solver | Counterfactual Regret Minimization and game-theoretic research |
| Training | Self-play, agent improvement, and reproducible experiments |
| Poker Arena | Human-versus-AI and AI-versus-AI gameplay |
| Analytics | Benchmarks, statistics, and strategy visualization |

## Development Roadmap

Development follows incremental, independently testable milestones.

| Version | Milestone | Status |
|---|---|---|
| v0.1 | Poker Core & Hand Evaluator | In progress |
| v0.2 | Equity Calculator | Planned |
| v0.3 | Heads-Up Game Engine | Planned |
| v0.4 | First AI Arena (CLI) | Planned |
| v0.5 | CFR Research Lab | Planned |
| v0.6 | Self-Play Training | Planned |
| v0.7+ | Visual Arena & Advanced Research | Planned |

See [Development Roadmap](docs/ROADMAP.md) for detailed milestones and acceptance criteria.

## Technology Stack

**Core development**
- C++20
- CMake
- C++ Standard Library
- Catch2 for automated testing

**Development environment**
- Ubuntu / WSL
- GCC
- Visual Studio Code
- Git and GitHub
- GitHub Actions for continuous integration

**Planned technologies**
- Python for experiments and data analysis
- React and TypeScript for a future web interface

## Engineering Principles

1. **Correctness first:** Algorithms and game rules must be thoroughly validated.
2. **Modular architecture:** Components should have clear responsibilities and controlled dependencies.
3. **Reproducibility:** Experiments must support deterministic execution where appropriate, controlled randomness, and documented configurations.
4. **Performance through measurement:** Optimize based on benchmarks and profiling.
5. **Incremental development:** Every milestone should produce working, testable functionality.
6. **Algorithmic understanding:** Core algorithms are implemented and studied rather than treated as black boxes.

## Research Direction

The long-term research focus includes:

- Exact and approximate poker equity computation
- Counterfactual Regret Minimization (CFR)
- Strategy approximation in imperfect-information games
- Reinforcement learning and self-play
- Agent evaluation and exploitative strategies
- Computational efficiency and algorithmic optimization

Simplified poker games, such as Kuhn Poker and Leduc Poker, may be used as isolated experiments to validate game-solving algorithms.

Solving full-scale No-Limit Texas Hold'em is not an initial objective.

## Documentation

- [Development Roadmap](docs/ROADMAP.md) — Milestones, deliverables, and acceptance criteria.
- [Software Architecture](docs/ARCHITECTURE.md) — Design principles, module boundaries, and architectural decisions.

## License

Distributed under the [MIT License](LICENSE).

Copyright © 2026 Arian Mohammadian.