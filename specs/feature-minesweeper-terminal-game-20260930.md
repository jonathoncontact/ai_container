# Feature: Small Terminal Minesweeper Game

## Feature Description

Implement a small, self-contained Minesweeper game in C++ that runs in the terminal. The player should see a concealed grid, reveal cells, place or remove flags, and win by revealing every non-mine cell before detonating a mine. The implementation should fit the repository's existing container-friendly C++ workflow and use only the standard library.

The initial version will use a fixed compact beginner board: 9 columns by 9 rows with 10 mines. A text interface is appropriate because the repository contains no graphical framework, application server, or UI toolkit.

## User Story

As a terminal user
I want to play a small Minesweeper board by entering commands
So that I can complete a quick puzzle without installing extra dependencies.

## Problem Statement

The repository currently contains only an empty `main.cpp` and no playable application or reusable game logic. There is no domain model, input loop, board rendering, win/loss handling, or automated test coverage for a game.

## Solution Statement

Create a reusable Minesweeper model separated from the terminal interface. The model will own the fixed 9x9 board, mine placement, adjacent-mine counts, revealed/flagged state, and game status. The CLI will render the model, parse a small command set, report invalid input, and continue until the player wins, loses, or quits. The first reveal will always be safe, normal games will use random mine placement, and model tests will inject explicit mine coordinates rather than depending on implementation-specific random sequences.

## Relevant Files

- `main.cpp` — currently the application entry point; replace the empty `main()` with CLI startup and the command loop.
- `test_runner.sh` — currently compiles every root-level `.cpp` and runs the application; update it so game unit and CLI tests can be run explicitly without treating test files as application entry points.
- `README.md` — document how to build, run, and play the game within the existing `cpp-container` workflow.
- `specs/README.md` — establishes that feature requirements live in `specs/`; no change is required unless the project wants an index.
- `.vscode/c_cpp_properties.json` and `.vscode/settings.json` — existing compiler conventions; use a standard supported C++ version and warning flags compatible with this setup.

### New Files

- `minesweeper.h` — public model types and `MinesweeperGame` interface.
- `minesweeper.cpp` — board initialization, mine placement, adjacent-count calculation, reveal/flag actions, flood reveal, and status transitions.
- `tests/minesweeper_tests.cpp` — deterministic unit tests for model behavior using a lightweight assertion helper or the project's chosen standard-library-only test style.
- `tests/test_cli.sh` — scripted smoke/integration test that launches the executable, sends valid and invalid commands, and verifies key output/status behavior.

## Implementation Plan

### Phase 1: Foundation

Define the game contract and data representation before adding terminal behavior. Establish one-based user-facing coordinates, zero-based internal indices, cell state, game status, action results, fixed dimensions, and fixed mine count. Keep the model independent of `std::cin`/`std::cout` so it can be tested directly.

Add the first deterministic unit tests before or alongside the implementation for board dimensions, mine-count constraints, and adjacent-mine counts. Preserve the simple root-level compilation workflow rather than introducing a third-party framework or a large build-system migration.

### Phase 2: Core Implementation

Implement mine placement, count calculation, reveal behavior, flag toggling, flood-fill expansion for zero-adjacent cells, loss handling, and win detection. Defer mine placement until the first reveal so that the selected first cell is guaranteed safe. Allow flags before the first reveal; preserve them while initializing the board, and reject an attempt to reveal a flagged cell until it is unflagged.

Cover both ordinary cells and boundary/corner cells. Make repeated actions idempotent or return a clear rejected-action result, and prevent actions after the game has ended.

### Phase 3: Integration

Build the terminal renderer and command parser around the model. Display one-based row/column coordinates, conceal unrevealed cells, distinguish flags, show revealed numbers, and reveal mines after a loss. Support commands such as `r <row> <col>` (reveal), `f <row> <col>` (toggle flag), `h` (help), and `q` (quit). Reject extra command tokens, validate coordinates and command syntax without terminating the process on malformed input, and treat EOF as a clean quit.

Update the runner and README, then validate model behavior, CLI behavior, clean compilation, and a complete win/loss flow.

## Step by Step Tasks

### Task 1: Confirm gameplay contract and add model test scaffolding

- Record the confirmed gameplay contract: fixed 9x9 board with 10 mines, safe first reveal, one-based coordinates, flags allowed before the first reveal, strict extra-token rejection, and EOF as a clean quit.
- Create `tests/minesweeper_tests.cpp` with a small test entry point and helpers for deterministic game construction.
- Add test cases for fixed dimensions, legal mine count, coordinate bounds, initial hidden state, and pre-reveal flag behavior.

**Acceptance criteria:** The fixed gameplay contract is written down in the spec and tests express the expected behavior without depending on random mine placement.

**Dependencies:** None.

**Estimated scope:** Small (1 new test file).

### Task 2: Implement the board model and deterministic mine setup

- Add `minesweeper.h` and `minesweeper.cpp` with cell state, game status, coordinate/action result types, and `MinesweeperGame`.
- Validate the fixed board configuration and ensure exactly 10 unique mines are placed within the 9x9 board after the first reveal.
- Provide a test-only construction path or board-layout injection that accepts explicit mine coordinates; do not make tests depend on standard-library RNG sequences.
- Compute adjacent mine counts from the final mine layout while preserving any pre-reveal flags.
- Expose read-only cell/status inspection needed by tests without exposing mutable board internals.

**Acceptance criteria:** Unit tests can create a known board, inspect safe/mine behavior through public operations, and no model code performs terminal I/O.

**Dependencies:** Task 1.

**Estimated scope:** Medium (3 files including tests).

### Task 3: Implement reveal, flag, and game-state transitions

- Implement reveal of a single cell, including safe first-reveal initialization, mine detonation, and loss status.
- Implement zero-cell flood reveal without recursion-depth risk; reveal only connected safe cells and their numbered boundary cells.
- Implement flag toggling before and after initialization, reject flags on revealed cells, and prevent revealing flagged cells until they are unflagged.
- Detect a win when every non-mine cell is revealed; reject or clearly report actions after win/loss.
- Add unit tests for corners/edges, safe first reveal, pre-reveal flags, flood reveal, flags, repeated actions, loss, win, and out-of-bounds coordinates.

**Acceptance criteria:** All core state transitions are deterministic and covered by tests, including both terminal outcomes and invalid actions.

**Dependencies:** Task 2.

**Estimated scope:** Medium (2 implementation/test files).

### Checkpoint: Core model

- [ ] Unit tests pass for board generation and all state transitions.
- [ ] The model builds with warnings enabled.
- [ ] No CLI concerns leak into the model API.

### Task 4: Add the terminal renderer and command loop

- Replace the empty `main()` with a CLI that constructs a beginner game and prints instructions plus the initial board.
- Render coordinates and cell symbols consistently; conceal mines until loss, show flags distinctly, and show all mines after loss.
- Parse `r`, `f`, `h`, and `q` commands with whitespace-tolerant tokenization.
- Report malformed commands, invalid coordinates, rejected actions, and current remaining mine/flag information without crashing.
- Keep input/output code separated into small functions so it remains testable and readable.

**Acceptance criteria:** A user can start a game, reveal and flag cells, receive useful errors for bad input, and reach win/loss/quit states from the terminal.

**Dependencies:** Task 3.

**Estimated scope:** Medium (main application plus model integration).

### Task 5: Add CLI integration coverage and update project documentation

- Add `tests/test_cli.sh` or an equivalent scripted test that feeds commands through stdin and checks startup, invalid-input handling, extra-token rejection, one-based coordinate handling, flagging, help, EOF, and quit. Keep deterministic win/loss coverage in model tests; do not add a user-facing seed or hidden test-layout mode.
- Update `test_runner.sh` with separate application and test commands, `set -euo pipefail`, and explicit source lists; avoid compiling a second `main()` into the application.
- Update `README.md` with build/run instructions, command syntax, one-based coordinate conventions, pre-reveal flag behavior, EOF behavior, and test instructions for local/container use.

**Acceptance criteria:** The documented commands work in the repository's container workflow, and the CLI smoke test proves the main user flow without manual interaction.

**Dependencies:** Task 4.

**Estimated scope:** Medium (runner, README, CLI test).

### Task 6: Run final validation and perform a focused review

- Compile the application with strict warnings and the same standard used by the repository's compiler configuration.
- Run all unit and CLI tests.
- Manually exercise at least one reveal, flag toggle, invalid command, quit, win, and loss flow.
- Review ownership, bounds checks, randomization, output clarity, and the absence of generated binaries in version control.

**Acceptance criteria:** Every validation command succeeds, the application exits cleanly, and no unrelated files are modified.

**Dependencies:** Task 5.

**Estimated scope:** Small (verification only).

## Testing Strategy

### Unit Tests

- Constructor validation for dimensions and mine count.
- Exact mine count and no duplicate mine positions.
- Correct adjacent-mine counts for center, edge, and corner cells.
- Reveal of safe, numbered, zero, and mine cells.
- Flood reveal boundaries and preservation of flagged cells.
- Flag toggling and rejection of illegal actions.
- Win/loss status transitions and post-game action rejection.
- Coordinate bounds and repeated commands.

### CLI / End-to-End Tests

- Startup renders a concealed board and usage instructions.
- `h` prints help; `q` exits cleanly.
- Malformed commands and out-of-range coordinates produce an error and keep the game running.
- Reveal and flag commands update visible output.
- Model-level fixtures allow deterministic win/loss verification without relying on CLI randomness or a user-facing test mode.

### Edge Cases

- The fixed 9x9 board with 10 mines.
- A first reveal on a cell that would otherwise have been a mine.
- Flags placed before the first reveal, including a flag on the eventual first-reveal cell.
- All safe cells revealed except one.
- Flood reveal adjacent to flags and board boundaries.
- Attempting to reveal a flagged cell, flag a revealed cell, or act after game over.
- EOF on stdin and extra whitespace/tokens.

## Acceptance Criteria

- [ ] The repository contains a playable terminal Minesweeper game with a fixed 9x9/10-mine beginner configuration.
- [ ] The board displays hidden, revealed-number, flagged, and mine states clearly.
- [ ] Reveal, flag toggle, help, quit, invalid-input, win, and loss behavior are implemented.
- [ ] Mine placement and adjacent counts are correct, with deterministic tests available.
- [ ] Revealing a zero cell expands safely to the appropriate neighboring area.
- [ ] The game does not crash on malformed commands, invalid coordinates, repeated actions, EOF, or post-game input.
- [ ] Unit tests deterministically cover win/loss and CLI integration tests cover parsing/rendering without relying on random board layout.
- [ ] `README.md` explains how to build and play the game in the existing container workflow.
- [ ] No third-party dependency is required.

## Validation Commands

Run from the repository root after implementation:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion main.cpp minesweeper.cpp -o app
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion minesweeper.cpp tests/minesweeper_tests.cpp -o minesweeper_tests
./minesweeper_tests
./tests/test_cli.sh ./app
```

The project runner should also expose a single documented command, for example:

```bash
./test_runner.sh
```

If implementation stays inside the existing container workflow, run the same commands from the `cpp-container` shell described in `README.md`.

## Notes

- The repository has no existing formal test framework, CMake project, or source directory convention. Keep the first implementation small and standard-library-only; a future change can introduce CMake or a test framework if the project grows.
- The normal CLI intentionally has no seed or hidden test-layout option. Deterministic board fixtures belong in model tests.
- First-reveal safety requires deferred mine placement. If the player flags the first selected cell, the CLI should report that the cell must be unflagged before it can be revealed.

## Open Questions

None for the agreed v1 scope.

## Plan Review

### Review Status

The plan is implementable for the current repository. The repository is intentionally minimal, so no external package or framework is needed. The standard library and shell-based tests are appropriate for the existing container workflow. The six gameplay decisions below have been confirmed and incorporated.

### Findings Addressed

1. **Known-board testing:** addressed with explicit mine-coordinate injection for model tests.

2. **First-reveal safety:** confirmed on; mine placement is deferred and pre-reveal flags are preserved.

3. **CLI win/loss automation:** the CLI remains random; deterministic win/loss coverage is provided by model tests.

4. **The test runner:** will use `set -euo pipefail`, separate application/test builds, explicit source lists, and one default command that runs both unit and CLI tests.

5. **Public inspection APIs:** the model will provide read-only status/cell inspection needed by tests, without exposing mutable internals.

6. **CLI input semantics:** extra tokens are rejected and EOF is a clean quit.

### Confirmed Defaults

- Fixed 9x9 board with 10 mines for the first release; no command-line difficulty configuration.
- One-based row/column coordinates (`1..9`) because they are friendlier for terminal users; convert to zero-based indices at the CLI boundary.
- First reveal is safe, matching common Minesweeper expectations.
- Flags are allowed before the first reveal and are preserved when the board is initialized; the first reveal cell itself must be unflagged or the command returns a clear rejection.
- Explicit mine-coordinate injection for model tests; no hidden test mode in the user-facing CLI.
- CLI integration tests cover parsing/rendering and quit/error paths; deterministic win/loss coverage lives in model tests.
- `q` and EOF both end the session with a normal exit; malformed commands do not end the session.
- Extra command tokens are rejected rather than silently ignored.

### Review Checklist for Implementation

- [x] The final model API makes deterministic board fixtures possible without exposing mutable internals.
- [x] The chosen first-reveal policy is reflected consistently in initialization, flags, tests, and README instructions.
- [x] CLI tests do not depend on incidental random output.
- [x] The runner builds application and tests separately and has no executable/test artifact collision.
- [x] README command examples match the parser exactly, including coordinate numbering and EOF behavior.
