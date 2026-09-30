# Terminal Minesweeper

This repository contains a small terminal Minesweeper game written in standard C++17. It uses a fixed 9x9 beginner board with 10 mines.

## Getting Started

This repository is compatible with [cpp-container](https://github.com/ChicoState/cpp-container). If not already built on your machine, clone and build it.

Run the container:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container
```

Run the application interactively in a shell:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container sh
```

Run the complete build and test workflow from the repository root:

```bash
./test_runner.sh
```

Build and run the application directly:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion \
  main.cpp minesweeper.cpp -o app
./app
```

Generated binaries from `test_runner.sh` are placed under `build/`, which is ignored by git.

## How to Play

The game uses one-based row and column coordinates from 1 through 9:

```text
r <row> <column>  reveal a cell
f <row> <column>  toggle a flag
h                 show help
q                 quit
```

The first successful reveal is always safe. Flags can be placed before the first reveal, but a flagged cell must be unflagged before it can be revealed. Invalid commands, invalid coordinates, and extra tokens are reported without ending the session. EOF exits normally.

## Tests

The model tests use explicit mine layouts for deterministic coverage of board rules, flooding, flags, win, and loss. The CLI smoke test validates command parsing and rendering without depending on a random board layout.

```bash
./test_runner.sh
```

## Structure

* `main.cpp` - terminal renderer and command loop
* `minesweeper.h`, `minesweeper.cpp` - game model and rules
* `.agents` - AI agent configurations and skills
* `specs` - feature plans and specifications
* `tests` - model and CLI tests
* `test_runner.sh` - build and test entry point
