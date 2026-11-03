# ♠ Poker AI Lab

**Building poker engines, solving imperfect-information games, and training intelligent agents from scratch.**

Poker AI Lab is a long-term software engineering and artificial intelligence project focused on **Heads-Up No-Limit Texas Hold'em**, primarily developed in C++20.

The goal is to build a modular platform where poker agents can play, learn through self-play, compete against each other, and be evaluated through reproducible experiments.

The project explores the intersection of **algorithms, probability, game theory, reinforcement learning, and high-performance software engineering**.

## Vision

Poker AI Lab is designed around three main capabilities:

- **Play:** Compete against custom poker agents and observe AI-versus-AI matches.
- **Train:** Develop increasingly sophisticated agents and investigate learning through self-play.
- **Analyze:** Calculate equity, investigate strategies, study imperfect-information game solving, and evaluate agent performance.

The long-term objective is not merely to build a poker game, but to create an environment for developing and understanding poker AI.

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

| Version | Milestone |
|---|---|
| v0.1 | Hand Evaluator |
| v0.2 | Equity Calculator |
| v0.3 | Heads-Up Game Engine |
| v0.4 | First AI Arena (CLI) |
| v0.5 | CFR Research Lab |
| v0.6 | Self-Play Training |
| v0.7+ | Visual Arena and Advanced Research |

See `docs/ROADMAP.md` for detailed milestones and acceptance criteria once available.

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

**Planned technologies**
- Python for experiments and data analysis
- React and TypeScript for the future web interface

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

## Project Status

**Current stage: Planning and architecture design.**

No implementation has been released yet.

The first development milestone will focus on a reliable hand evaluator for Texas Hold'em, followed by an equity calculator supporting exact enumeration and Monte Carlo simulation.

## Documentation

As the project evolves, technical documentation, architecture decisions, benchmarks, and experimental results will be added.

## License

License to be selected before the first public release.
