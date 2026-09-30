# Spec: Terminal Minesweeper Game

## Objective

Build a small, self-contained terminal Minesweeper game in C++ for a user playing from a shell. The game provides a fixed beginner board of 9 rows by 9 columns with 10 mines. The player can reveal cells, toggle flags, request help, quit, and receive clear feedback for invalid input.

The game is complete when a player can start a session, safely make the first reveal, play until winning or detonating a mine, and run the automated model and CLI tests without external dependencies.

### Confirmed assumptions

- The interface is terminal-based; no GUI or web layer is required.
- The board is fixed at 9x9 with 10 mines for v1.
- The first reveal is always safe.
- User-facing coordinates are one-based (`1..9`); internal indices may be zero-based.
- Flags may be placed before the first reveal and are preserved when mines are initialized.
- A flagged cell cannot be revealed until it is unflagged.
- Extra command tokens are invalid rather than ignored.
- EOF is treated as a clean quit.
- Normal CLI play is random; deterministic win/loss tests use explicit model test fixtures, not a hidden CLI mode.
- Only the C++ standard library and shell tools are used.

## Tech Stack

- C++17 or the repository's supported equivalent.
- Standard library only: containers, algorithms, random generation, streams, and error handling.
- POSIX-compatible shell for `test_runner.sh` and CLI smoke tests.
- No third-party test framework is required; tests may use a small local assertion helper.

## Commands

Run these commands from the repository root after implementation:

```bash
# Build the playable application
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion \
  main.cpp minesweeper.cpp -o app

# Build model tests
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion \
  minesweeper.cpp tests/minesweeper_tests.cpp -o minesweeper_tests

# Run model tests
./minesweeper_tests

# Run CLI smoke tests against a supplied application binary
./tests/test_cli.sh ./app

# Run the repository's complete build-and-test workflow
./test_runner.sh

# Start an interactive game
./app
```

The existing container workflow remains supported:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container sh
```

Then run the repository commands from `/usr/src`.

## Project Structure

```text
main.cpp                         CLI entry point, renderer, and command loop
minesweeper.h                    Public game model types and interface
minesweeper.cpp                  Board state and game rules
tests/minesweeper_tests.cpp      Deterministic model/unit tests
tests/test_cli.sh                Scripted CLI integration/smoke test
test_runner.sh                   Explicit application and test build runner
README.md                        Build, play, and test documentation
specs/minesweeper-game-spec.md   This specification
```

The model must not depend on terminal input/output. The CLI owns `std::cin`/`std::cout` and translates one-based user coordinates at the boundary.

## Functional Requirements

### Board and initialization

- The game board is exactly 9 rows by 9 columns.
- Exactly 10 unique mine positions are selected.
- Mines are initialized lazily on the first successful reveal so that the selected cell is not a mine.
- Adjacent mine counts are calculated after mine placement.
- Pre-reveal flags remain intact through initialization.
- A flagged first cell cannot be revealed; the player must unflag it first.

### Commands

| Command | Behavior |
|---|---|
| `r <row> <col>` | Reveal an unflagged cell using one-based coordinates. |
| `f <row> <col>` | Toggle a flag on an unrevealed cell. |
| `h` | Print command help. |
| `q` | Quit normally. |

Commands with missing arguments, non-numeric/out-of-range coordinates, unknown verbs, or extra tokens must print an error and continue the session. EOF must exit normally.

### Cell visibility and outcomes

- Hidden cells are concealed.
- Flagged cells are visibly distinct from hidden cells.
- Revealed safe cells show their adjacent mine count; zero cells trigger iterative flood reveal of connected safe cells and their numbered boundary cells.
- Revealing a mine changes the game to loss and reveals mines in the final board.
- Revealing all non-mine cells changes the game to win.
- Actions after win or loss are rejected with a clear message.
- Repeated reveal/flag actions have defined, non-crashing results.

## Code Style

Use small types and explicit results rather than exposing mutable board data. Names should be descriptive and use `PascalCase` for types and `camelCase` for functions/variables. Keep formatting compatible with the existing compiler settings and enable warnings during validation.

Example model-facing style:

```cpp
enum class GameStatus { Ready, InProgress, Won, Lost };

struct Position {
    int row;
    int column;
};

struct CellView {
    bool revealed;
    bool flagged;
    bool mine;
    int adjacentMines;
};

struct ActionResult {
    bool accepted;
    bool changed;
    GameStatus status;
    const char* message;
};

class MinesweeperGame {
public:
    static constexpr int kRows = 9;
    static constexpr int kColumns = 9;
    static constexpr int kMineCount = 10;

    ActionResult reveal(Position position);
    ActionResult toggleFlag(Position position);
    CellView cellAt(Position position) const;
    GameStatus status() const;
};
```

Implementation conventions:

- Validate all coordinates at public boundaries.
- Use an iterative queue/stack for flood reveal rather than recursive traversal.
- Return explicit action results for success, rejection, and terminal transitions.
- Keep random generation behind the model implementation; provide explicit mine-coordinate injection or a test factory for deterministic tests.
- Do not expose mutable references to internal cells.
- Avoid third-party dependencies, global mutable state, and terminal I/O inside the model.

## Testing Strategy

### Model/unit tests

`tests/minesweeper_tests.cpp` must use explicit mine layouts so test outcomes do not depend on a standard-library RNG sequence. Cover:

- Fixed dimensions and exactly 10 unique mines.
- Initial hidden state and pre-reveal flag preservation.
- Safe first reveal, including deterministic relocation when a test fixture's candidate mine set contains the requested first cell.
- Correct adjacent counts for center, edge, and corner cells.
- Zero-cell flood reveal and numbered boundary cells.
- Flag toggling, flagged-cell reveal rejection, and revealed-cell flag rejection.
- Win, loss, repeated actions, post-game actions, and out-of-bounds positions.

### CLI integration tests

`tests/test_cli.sh` must execute the compiled app with scripted stdin and verify stable text markers for:

- Startup board and help text.
- Help and quit commands.
- One-based coordinate handling.
- Flagging and unflagging.
- Invalid commands, invalid coordinates, and extra-token rejection.
- EOF clean exit.

The CLI test must not depend on the random board layout or require a hidden seed/test option. Model tests own deterministic win/loss coverage.

### Verification expectations

- All compilation commands complete without errors.
- Unit tests and CLI tests exit successfully.
- The application does not crash on malformed input, EOF, repeated actions, or post-game commands.
- No generated executable is required to be committed.

## Boundaries

### Always do

- Keep the model and CLI responsibilities separate.
- Validate coordinates, command arity, and input conversion.
- Preserve the fixed v1 gameplay contract.
- Add or update tests for every rule change.
- Run the complete documented validation workflow before declaring implementation complete.
- Keep the README synchronized with actual commands and coordinate behavior.

### Ask first

- Adding external libraries or a test framework.
- Changing the board size, mine count, command syntax, or coordinate convention.
- Adding CLI difficulty selection, a seed option, or a hidden test mode.
- Introducing a build system or changing the container workflow.
- Changing the scope from terminal UI to graphical/web UI.

### Never do

- Do not expose mutable board internals to the CLI or tests.
- Do not make tests depend on incidental random-number generator output.
- Do not silently ignore malformed commands or extra tokens.
- Do not remove or weaken tests to make the build pass.
- Do not commit generated binaries, secrets, or unrelated workspace changes.

## Success Criteria

- [ ] `./app` starts a playable fixed 9x9/10-mine terminal game.
- [ ] The first successful reveal is never a mine.
- [ ] One-based coordinates work consistently in commands and displayed instructions.
- [ ] Flags work before and after initialization and block reveals until removed.
- [ ] Zero cells expand iteratively and safely.
- [ ] Win and loss are correctly detected and displayed.
- [ ] Mines are shown after loss and hidden during active play.
- [ ] Invalid commands, invalid coordinates, extra tokens, EOF, repeated actions, and post-game input are handled without crashes.
- [ ] Model tests deterministically cover board rules and win/loss behavior.
- [ ] CLI smoke tests cover parsing, rendering, flagging, help, errors, and quit behavior.
- [ ] `./test_runner.sh` runs the documented build-and-test workflow successfully.
- [ ] `README.md` documents build, play, and test instructions.
- [ ] No third-party dependency is required.

## Open Questions

None for the agreed v1 scope. Any change to the confirmed assumptions is a scope change and should update this specification before implementation.
