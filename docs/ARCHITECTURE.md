# Poker AI Lab — Software Architecture

**Status:** Initial architecture proposal  
**Primary language:** C++20  
**Primary game:** Heads-Up No-Limit Texas Hold'em  
**Architecture approach:** Modular monolith, incremental development

## 1. Architectural Goals

Poker AI Lab aims to provide a reliable foundation for poker simulation, AI research, and interactive gameplay.

The architecture should support:

- Correct and independently testable poker algorithms.
- Reusable game logic shared by agents and applications.
- Strict separation between hidden game state and player observations.
- Reproducible simulations and experiments.
- Efficient execution of large numbers of poker hands.
- Gradual extension without premature abstraction.

Maintainability and correctness take priority over speculative extensibility.

## 2. Architectural Style

Poker AI Lab will initially use a **modular monolith**.

All components will live in one Git repository, primarily written in C++20. Modules will be introduced as their responsibilities become concrete.

The project will not initially use microservices, distributed infrastructure, plugin frameworks, or a complex dependency injection system.

A modular monolith allows us to share core logic, simplify testing, and evolve component boundaries as our understanding improves.

## 3. Conceptual Components

### 3.1 Poker Core

Responsible for fundamental poker data structures and algorithms.

Planned responsibilities:

- Card rank and suit representation.
- Standard 52-card deck.
- Hand evaluation and comparison.
- Deterministic randomization where appropriate.

Poker Core must not depend on agents, training, user interfaces, or the game engine.

### 3.2 Equity Calculator

Responsible for calculating outcome probabilities from known cards and future-card uncertainty.

Planned responsibilities:

- Exact outcome enumeration.
- Monte Carlo simulation.
- Tie and split-pot accounting.
- Controlled random sampling.
- Future support for hand ranges.

The calculator depends on Poker Core but does not manage betting rounds or game state.

### 3.3 NLHE Game Engine

Responsible for the authoritative state and rules of Heads-Up No-Limit Texas Hold'em.

Planned responsibilities:

- Player positions and blinds.
- Betting rounds and legal actions.
- Stack and pot management.
- State transitions.
- All-in and showdown resolution.
- Public and private game information.
- Deterministic test scenarios.

The engine should not depend on concrete agents, user interfaces, or learning algorithms.

### 3.4 AI Agents

Responsible for selecting actions based on permitted observations.

Potential agent types:

- Random legal-action agents.
- Rule-based agents.
- Equity-based heuristic agents.
- Learned policies.
- Agents using solver-generated strategies.

Agents must not have access to hidden opponent cards or internal engine state beyond their allowed observations.

### 3.5 Poker Solver

Responsible for studying approximate equilibrium strategies in imperfect-information games.

Research topics include:

- Counterfactual Regret Minimization.
- Regret and average-strategy updates.
- Information sets.
- Strategy convergence.
- Exploitability and best-response evaluation.
- Simplified Hold'em subgames.

Kuhn Poker and potentially Leduc Poker will serve as isolated validation environments.

The solver is not required to depend on the complete NLHE game engine.

### 3.6 Training

Responsible for orchestrating agent learning experiments.

Planned responsibilities:

- Self-play execution.
- Training configurations.
- Agent checkpoints.
- Evaluation against fixed opponents.
- Experiment logging and reproducibility.

Training algorithms and data formats will be selected when their practical requirements are understood.

### 3.7 Poker Arena

Responsible for interaction with the game and presentation of matches.

Planned interfaces:

- Command-line interface.
- Automated agent-versus-agent matches.
- Future graphical web application.

The Arena must use the authoritative game engine rather than duplicate game rules.

### 3.8 Analytics

Responsible for measuring and visualizing results.

Planned capabilities:

- Game and match statistics.
- Agent performance comparisons.
- Experiment summaries.
- Benchmarks.
- Strategy analysis and visualization.

Analytics must not influence authoritative game-state transitions.

## 4. Dependency Rules

The following dependency principles apply:

1. Poker Core has no dependencies on higher-level project components.
2. Equity Calculator depends on Poker Core.
3. The NLHE Game Engine depends on Poker Core.
4. Agents interact with the engine through permitted observations and legal-action contracts.
5. Training orchestrates game execution and agent interaction.
6. Arena applications consume engine functionality without reimplementing rules.
7. The solver may use independent game abstractions for simplified research environments.
8. Analytics consumes results and recorded data rather than controlling game rules.

Dependencies should remain explicit and preferably unidirectional.

The exact set of CMake targets and library boundaries will be defined as implementation requirements emerge.

## 5. Game State and Information Boundaries

Poker is an imperfect-information game. The architecture must explicitly distinguish between:

**Authoritative Game State**

The complete state required to execute a legal game, including private player cards, remaining deck information, stacks, betting history, and the current stage.

**Player Observation**

The information a player or agent is permitted to know, including:

- Their own hole cards.
- Community cards.
- Public betting history.
- Player positions.
- Visible stack and pot information.
- Available legal actions.

Opponent hole cards and undealt cards must not be exposed.

Agent decisions must be based on observations, not unrestricted references to authoritative game state.

## 6. State Transitions

The game engine should own and validate all state transitions.

Conceptually:

    GameState + PlayerAction
                |
                v
         Action Validation
                |
                v
          State Transition
                |
                v
          Updated GameState

The engine determines whether actions are legal and applies their effects.

No agent or UI component may directly modify authoritative game state.

Action validation and transition functions must preserve the game's invariants.

## 7. Core Invariants

Examples of invariants to preserve include:

- A physical card cannot appear more than once in a valid deal.
- Card identities remain valid throughout their lifetime.
- Betting actions must satisfy applicable NLHE rules.
- Player stacks and pot amounts cannot become negative.
- Total chips are conserved during a hand, including settlement.
- Showdown outcomes follow the hand evaluator's ordering.
- Agents receive only information they are permitted to observe.
- Randomized experiments can be reproduced when their seeds, configurations, and execution conditions are controlled.

Additional invariants will be defined for each component during implementation.

## 8. Initial Repository Organization

The initial repository will use a minimal structure:

    poker-ai-lab/
    ├── include/
    │   └── poker/
    ├── src/
    ├── tests/
    ├── apps/
    ├── docs/
    │   ├── ROADMAP.md
    │   └── ARCHITECTURE.md
    ├── CMakeLists.txt
    ├── README.md
    ├── .gitignore
    └── LICENSE

Folders and source files will be added as they become necessary.

Future components such as solvers, training frameworks, analytics tools, or web applications will not be scaffolded prematurely.

## 9. Build and Tooling

The initial development environment will use:

- C++20.
- CMake.
- GCC on Ubuntu / WSL.
- Catch2 for testing.
- Git and GitHub.

The build system should:

- Require C++20 without compiler-specific language extensions.
- Support automated test execution through CTest.
- Separate library code from executable applications.
- Support warning and sanitizer configurations during development.
- Avoid machine-specific absolute paths.
- Work across the project's supported development devices.

Python and web technologies will be added only when required.

## 10. Performance and Optimization

Performance-sensitive algorithms may eventually require specialized representations or data structures.

Examples include:

- Compact card identifiers.
- Bitmask-based evaluation.
- Precomputed lookup tables.
- Optimized Monte Carlo sampling.
- Parallel simulation.
- Memory-efficient strategy representations.

Such optimizations should be introduced only after correctness has been established and benchmarks identify meaningful bottlenecks.

Public interfaces should not expose implementation details unnecessarily.

## 11. Testing Strategy

Testing will progress from individual algorithms to complete systems.

**Unit testing**

Validate components, edge cases, and invariants.

**Exhaustive testing**

Use exhaustive enumeration when the state space is sufficiently small, particularly for five-card hand evaluation.

**Property-based and randomized testing**

Verify properties such as card uniqueness, chip conservation, and consistent hand comparisons.

**Integration testing**

Test complete game sequences, agent interactions, and engine state transitions.

**Research validation**

Compare algorithmic outputs with known theoretical results, independent implementations, and reproducible experiments.

**Performance testing**

Record benchmark workloads, compiler settings, hardware context, and relevant measurements.

## 12. Architectural Decision Policy

Significant architectural changes should document:

- The problem being solved.
- Alternatives considered.
- The chosen solution.
- Relevant trade-offs.
- Consequences for existing components.

Small implementation details do not require formal architecture decision records.

The architecture should evolve based on actual requirements rather than speculative future features.

## 13. Initial Implementation Scope

The first implementation milestone is limited to:

- Card representation.
- Deck representation.
- Five-card evaluation.
- Best-five-of-seven evaluation.
- Hand comparison.
- Automated tests.
- A minimal demonstration executable.

The equity calculator, full NLHE game engine, agents, solver, training infrastructure, and graphical interface remain outside this initial implementation.

## 14. Architecture Evolution

The initial design is intentionally incomplete.

Future module boundaries, persistent formats, external APIs, learning architectures, and parallelization strategies will be decided only when their requirements become concrete.

The guiding principle is:

**Build the simplest correct system that supports the current milestone without unnecessarily restricting the project's long-term direction.**
