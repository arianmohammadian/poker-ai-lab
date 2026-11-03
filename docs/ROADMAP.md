# Poker AI Lab — Development Roadmap

**Project:** Poker AI Lab  
**Primary game:** Heads-Up No-Limit Texas Hold'em  
**Language:** C++20  
**Development strategy:** Incremental, test-driven where practical, and research-oriented

## 1. Project Objective

Build a modular poker AI platform capable of:

- Running correct and reproducible Texas Hold'em games.
- Evaluating hands and computing poker equity.
- Supporting human and AI-controlled players.
- Developing game-theoretic solvers.
- Training agents through self-play.
- Comparing agent performance through controlled experiments.
- Providing interactive gameplay and strategy analysis tools.

The project prioritizes algorithmic correctness, clear software architecture, reproducibility, and measurable progress.

Full-scale GTO solving for No-Limit Texas Hold'em is not a committed deliverable.

## 2. Development Milestones

### v0.1 — Poker Core: Hand Evaluation

**Goal:** Implement a correct and efficient Texas Hold'em hand evaluator.

**Scope**

- Define card ranks, suits, and card representation.
- Implement a standard 52-card deck.
- Support deterministic shuffling with controlled randomness.
- Evaluate and compare five-card poker hands.
- Determine the best five-card hand from seven cards.
- Represent hand strength and tie-breaking information.
- Provide an executable demonstration.

**Acceptance criteria**

- All ten standard five-card hand categories are recognized.
- Kickers and ties are handled correctly.
- Edge cases, including the ace-low straight, are tested.
- Duplicate or otherwise invalid card combinations are rejected at defined public API boundaries.
- Five-card evaluation is exhaustively validated using all 2,598,960 possible five-card combinations and their known category frequencies.
- Seven-card evaluation is tested against an independent reference or oracle.
- Unit tests pass, including checks under sanitizers.
- Basic evaluation performance is benchmarked.

**Deliverable:** A tested, reusable C++ hand-evaluation library.

---

### v0.2 — Equity Calculator

**Goal:** Calculate winning probabilities for Texas Hold'em hands.

**Scope**

- Represent player hole cards and community cards.
- Compute exact equity by enumerating possible remaining boards.
- Estimate equity using Monte Carlo simulation.
- Handle wins, losses, and split pots.
- Support at least two players with known hole cards.
- Provide configurable iteration counts and random seeds.
- Expose functionality through a command-line application.

**Acceptance criteria**

- Duplicate cards are rejected.
- Exact enumeration produces correct results on verified test cases.
- Monte Carlo estimates are compared against exact results.
- Randomized experiments can be reproduced with fixed seeds.
- Equity correctly accounts for split pots.
- Simulation performance is benchmarked and documented.

**Deliverable:** A command-line Texas Hold'em equity calculator.

**Deferred:** General weighted range-versus-range evaluation and large multiway analyses.

---

### v0.3 — Heads-Up NLHE Game Engine

**Goal:** Simulate complete legal Heads-Up No-Limit Texas Hold'em games.

**Scope**

- Represent players, stacks, pots, and game state.
- Implement blinds, dealer/button position, and action order.
- Implement preflop, flop, turn, and river transitions.
- Validate fold, check, call, bet, raise, and all-in actions.
- Handle betting rounds, minimum raises, and short all-in raises.
- Resolve showdowns, split pots, and pot distribution.
- Support deterministic game execution for testing.
- Expose observations without revealing hidden opponent cards.

**Acceptance criteria**

- Complete Heads-Up hands can be simulated.
- All generated and accepted actions comply with the implemented rules.
- Betting and all-in edge cases are tested.
- Chip conservation is verified across state transitions, including settlement.
- Showdown results agree with the hand evaluator.
- Hidden cards are not exposed to agents.
- Fixed-seed games are reproducible.
- Automated tests cover representative game sequences.

**Deliverable:** A standalone, tested Heads-Up NLHE engine.

**Deferred:** Multiway play, tournament structures, and advanced table management.

---

### v0.4 — First AI Arena

**Goal:** Play Texas Hold'em against simple AI agents.

**Scope**

- Define a minimal interface between game engine and agents.
- Implement a random legal-action agent.
- Implement at least one rule-based or heuristic agent.
- Build an interactive command-line game.
- Support automated agent-versus-agent matches.
- Record basic hand histories and match results.

**Acceptance criteria**

- A human can complete a Heads-Up match against an agent.
- Agents receive only permitted observations.
- Agents cannot submit illegal actions without explicit engine handling.
- Automated matches can run without human interaction.
- Results can be reproduced when random seeds and configurations are fixed.
- Match histories are inspectable for debugging.

**Deliverable:** The first playable Poker AI Lab experience.

---

### v0.5 — CFR Research Lab

**Goal:** Implement and validate Counterfactual Regret Minimization.

**Scope**

- Study extensive-form imperfect-information games.
- Implement vanilla CFR.
- Use Kuhn Poker as a small, independently testable research environment.
- Compare computed strategies against known theoretical properties.
- Extend experiments to Leduc Poker if justified.
- Investigate simplified Heads-Up Hold'em subgames.
- Measure convergence and approximate exploitability where computationally feasible.

**Acceptance criteria**

- The solver respects information sets and legal actions.
- Regret updates and average-strategy calculations are tested.
- Kuhn Poker results match known theoretical benchmarks within specified tolerances.
- Training results are reproducible.
- Convergence metrics are recorded and analyzed.
- At least one experiment demonstrates the applicability and limitations of CFR to a simplified Hold'em situation.

**Deliverable:** A validated CFR research implementation with documented experiments.

**Deferred:** Full-scale NLHE solving and highly optimized CFR variants.

---

### v0.6 — Self-Play Training

**Goal:** Investigate whether agents can improve through self-play.

**Scope**

- Define a suitable policy representation and learning method.
- Build a reproducible self-play training pipeline.
- Implement checkpoints and agent versioning.
- Support agent-versus-agent evaluation.
- Track performance against fixed baseline opponents.
- Analyze changes in strategic behavior.

**Acceptance criteria**

- Training runs can be configured and repeated.
- Agent checkpoints can be saved and loaded.
- Trained agents can play through the existing engine.
- Evaluation uses separate, controlled matches.
- Performance results include uncertainty estimates.
- Improvement claims are supported by statistical evidence rather than training win rate alone.

**Deliverable:** A self-play experimentation pipeline with measured agent performance.

**Research note:** The learning algorithm will be selected after evaluating practical alternatives, computational requirements, and the capabilities of the existing engine.

---

### v0.7 — Visual Poker Arena

**Goal:** Provide an interactive environment for gameplay and analysis.

**Scope**

- Create a graphical Heads-Up poker table.
- Display stacks, pot, cards, and legal actions.
- Allow human-versus-agent matches.
- Support replay and inspection of recorded hands.
- Show basic agent statistics and match results.
- Integrate with the existing C++ game engine.

**Acceptance criteria**

- The interface correctly represents engine state.
- Human actions are validated by the engine.
- Gameplay does not duplicate poker-rule logic.
- Recorded matches can be replayed consistently.
- The application is documented and demonstrable.

**Deliverable:** A graphical Poker AI Lab application.

**Tentative technologies:** React and TypeScript, subject to later architectural review.

---

## 3. Long-Term Research Directions

The following topics are exploratory rather than committed releases:

- CFR+ and alternative regret-minimization algorithms.
- Action and information abstraction.
- Depth-limited subgame solving.
- Improved hand-evaluation performance.
- Weighted range-versus-range analysis.
- More advanced reinforcement learning methods.
- Exploitative agent strategies.
- Parallel and accelerated simulations.
- Extension from Heads-Up to 6-max.
- Advanced strategy visualization and analytics.

These directions will be prioritized based on experimental results and the maturity of the existing platform.

## 4. Engineering Standards

Every implementation milestone should satisfy relevant quality requirements:

**Correctness**
- Automated tests for normal and edge cases.
- Clearly defined invariants and input validation.
- Independent reference checks when feasible.

**Maintainability**
- Small, focused components.
- Explicit dependencies.
- Clear ownership of game state and responsibilities.
- No unnecessary abstractions.

**Performance**
- Benchmark before optimizing.
- Record compiler settings and workload characteristics.
- Preserve correctness when introducing optimizations.

**Reproducibility**
- Control pseudorandom seeds.
- Record experiment configurations.
- Separate training and evaluation.
- Make relevant results reproducible where feasible.

**Documentation**
- Document public interfaces and non-obvious decisions.
- Update the roadmap when scope changes.
- Record benchmarks and research findings.

## 5. Definition of Done

A milestone is considered complete when:

1. Its required functionality is implemented.
2. Its acceptance criteria are satisfied.
3. Relevant automated tests pass.
4. No known correctness defects invalidate its results.
5. Performance is measured where relevant.
6. Documentation reflects the actual implementation.
7. The project builds successfully in the supported environment.
8. The milestone is integrated into the main repository.

Features outside the agreed scope do not block milestone completion.

## 6. Development Policy

- Complete functional milestones before expanding scope.
- Prefer simple designs that can evolve when necessary.
- Avoid premature optimization.
- Treat experimental results as evidence, not assumptions.
- Reassess future milestones as new technical constraints emerge.
- Keep the GitHub repository as the source of truth for implemented capabilities.

## 7. Current Status

**Current phase:** Project foundation and architecture planning.

**Next development milestone:** v0.1 — Poker Core: Hand Evaluation.

No implementation milestones have been completed yet.
