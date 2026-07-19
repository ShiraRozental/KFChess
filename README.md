# KFChess

A real-time chess game with modified rules, written in C++17 with an OpenCV-based view layer.

There are no turns: both players move at the same time, and every piece that acts must rest before it can act again. The game ends only when a king is captured.

## Modified rules

**Simultaneous play.** Black and White move concurrently. Nobody waits for anybody.

**No check or checkmate.** The game ends only when a king is actually captured. The player who captures the opposing king wins immediately.

**Cooldown (rest).** After any move or jump, a piece enters a rest state and cannot act at all until its cooldown expires. Cooldown durations come from configuration, not from game logic.

**Jump.** A piece can jump in place for a short cooldown, used defensively. Jump is always a manual command — it never fires automatically as a dodge. A piece is protected from capture *only* while mid-jump; once the jump cooldown ends it is subject to the ordinary collision rules like any other piece.

### Collision rules

**Opposite colors** — when two pieces of different colors end up on the same cell, whoever arrived *later* captures whoever was there first. This holds whether one piece moved or both did.

**Same color** — when two same-color pieces would occupy one cell, the piece arriving later stops one cell short instead of completing its move. It neither lands on nor passes through the occupied cell.

> Example: a rook moves e1→e8 while a friendly queen moves a4→h4. Their paths cross at e4, so the rook stops at e3.

**Movement lock** — a piece already mid-move cannot be redirected to a new destination until its current move finishes.

## Scoring

Score is the total value of captured enemy pieces.

| Piece | Value |
|-------|-------|
| Pawn | 1 |
| Knight | 3 |
| Bishop | 3 |
| Rook | 5 |
| Queen | 9 |
| King | immediate win (no points) |

Cooldowns currently live in [src/realtime/CooldownConfig.cpp](src/realtime/CooldownConfig.cpp): 1000 ms for every piece type, 500 ms for a jump. The lookup is already per-piece-type, so giving each piece its own duration is a one-file change.

## Prerequisites

- A C++17 compiler (MSVC / MinGW-w64)
- CMake 3.15 or newer
- OpenCV 4.5.1 — **not included in this repository**, see below

### OpenCV setup

The OpenCV binaries are too large for git, so they are excluded from the repo. Download the `OpenCV_451` folder from:

https://drive.google.com/drive/folders/14SeyjbNPvsgyLKM2omcVTlTX0wAQ-_Ox?usp=sharing

Place it under `third_party/` in the project root, so that these paths exist:

```
third_party/OpenCV_451/include/      # headers
third_party/OpenCV_451/bin/          # opencv_world451.lib, opencv_world451d.lib and matching DLLs
```

Without this step CMake fails at the include/link stage. On Windows the correct DLL is copied next to the executable automatically after each build.

Everything else needed to build ships with the clone: `nlohmann/json`, the doctest header, and all sprite and board assets under [assets/](assets/).

## Building and running

```bash
cmake -S . -B build
cmake --build build --config Debug
./build/Debug/KFChess
```

The game locates `assets/` by walking up from the executable, so it can be launched from any working directory.

Two executables are produced:

- **`KFChess`** — the graphical game ([app/KFChessApp.cpp](app/KFChessApp.cpp)), starting from a standard board setup.
- **`KFChessScript`** — a headless runner ([Main.cpp](Main.cpp)) that reads a board and a list of commands from stdin and prints the result. Useful for scripted scenarios and debugging without a window.

## Playing

Both players share one screen and one mouse.

| Input | Action |
|-------|--------|
| Left click | Select a piece, then click again to choose its destination |
| Right click | Make the selected piece jump in place |
| `ESC` | Quit |

Selecting a piece highlights its legal destinations. The window shows the board, each player's moves panel, live scores, and a game-over banner naming the winner.

## Project structure

| Directory | Responsibility |
|-----------|----------------|
| [include/model/](include/model/) | Board, pieces, positions, game state |
| [include/rules/](include/rules/) | Movement rules, reachability, legal destinations |
| [include/realtime/](include/realtime/) | Cooldowns, motion, real-time arbitration of simultaneous moves |
| [include/engine/](include/engine/) | Game engine, snapshots, event listeners, moves log, scoreboard |
| [include/input/](include/input/) | Controller, mouse routing, pixel-to-board mapping |
| [include/view/](include/view/) | Renderer abstraction, OpenCV image view, screen layout |
| [include/animation/](include/animation/) | Sprite timelines and per-piece animation state |
| [include/io/](include/io/) | Board parsing and printing, notation, formatting |
| [app/](app/) | Graphical application entry point |
| [tests/](tests/) | doctest unit tests and fixtures |

## Architecture

**The UI is fully decoupled from the game logic.** All view code lives under `src/view/`, and the engine talks to an abstract `Renderer` — it never knows OpenCV exists. The test build deliberately excludes view sources, so the entire test suite compiles and runs without OpenCV installed.

**Patterns in use.** Strategy for piece movement rules and piece representation; Factory for building rules per piece type; Observer for `MovesLog` and `ScoreBoard`, which subscribe to engine events rather than being polled; Command for parsed script actions.

**Nothing is hard-coded.** Cooldowns come from configuration, movement rules are injectable strategies, and board data is only reachable through the `Board` abstraction — never as raw state from outside. This keeps the door open for a binary board representation and for user-defined pieces later.

## Tests

```bash
cmake -S . -B build_tests -DBUILD_TESTS=ON
cmake --build build_tests
./build_tests/Debug/KFChessTests
```

Or via CTest: `ctest --test-dir build_tests`.

The suite uses [doctest](https://github.com/doctest/doctest), vendored under `tests/doctest` so tests build without network access. Coverage focuses on the tricky cases: simultaneous moves, cooldown interactions, collision resolution, and king capture.

For a coverage report (requires a gcov-capable toolchain and `gcovr`):

```bash
cmake -S . -B build_tests -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
cmake --build build_tests
./build_tests/KFChessTests
gcovr --html-details coverage/index.html
```

## Roadmap

- **Quadcopter** — a new piece reaching any square within ±2 rows and columns, with a longer cooldown
- **Per-piece cooldowns** — distinct rest durations per piece type
- **Binary board representation** — swapped in behind the existing abstraction
- **User-defined pieces** — movement rules and promotion behavior loaded from config
- **Scale** — support for thousands of concurrent games
