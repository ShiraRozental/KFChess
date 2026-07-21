# Chess Game Project (Modified Rules)

## Project Overview
A C++ chess game with modified rules, built iteratively.
Core game logic and the rendering/view layer are both in place. Current work is on the client/server split (see `SERVER_PLAN.md`).

## Project Layout
Headers live in `include/<area>/<module>/`; sources mirror them under `src/<area>/<module>/`. Four top-level areas:

- **`logic/`** — the game itself, with no UI and no networking.
  `model/` (Board, Piece, Position) · `rules/` (movement strategies, reachability) · `realtime/` (cooldowns, motion, arbiter) · `engine/` (GameEngine, scoring, moves log) · `io/` (board text parsing/printing, notation) · `bus/` (game event listeners, header-only) · `config/` (PieceDefinition, PieceCatalog)
- **`client/`** — anything that renders or accepts input.
  `view/` (Renderer, ImageView, sprites — **the only place OpenCV may be used**) · `animation/` · `input/` (Controller, mouse routing) · `app/` (entry point) · `net/`
- **`server/`** — `session/` · `rooms/` · `matchmaking/` · `accounts/`. Scaffolded, largely empty.
- **`shared/`** — used by both client and server: `protocol/` (WireProtocol) · `net/` · `common/`

Unit tests live in `tests/unit/`, one file per production file, named `<Thing>Test.cpp`. Script-driven tests live in `texttests/`.

### Placing a new file
Decide by **dependency direction**, not by topic:

- `logic/` must not include from `client/`, `server/`, or `shared/`
- `client/` and `server/` may include from `logic/` and `shared/`
- `shared/` may include from `logic/` only

If putting a file somewhere would force an include that breaks the above, it belongs in a different area. Header graphs must stay acyclic between areas; a `.cpp` reaching into a sibling area is the normal composition point (e.g. `logic/model/*.cpp` reading `logic/config/`).

**Exercise judgment on every new file rather than defaulting to the folder of whatever you edited last.** When two areas both look defensible, say which you'd pick and why, and confirm before creating it — moving a file after others include it is expensive.

CMake globs sources recursively, so a new directory is picked up automatically, but CMake must be re-run to notice it.

## Modified Game Rules
- **Simultaneous play**: Both Black and White move at the same time, no turns
- **No Check/Checkmate**: Game ends ONLY when a King is captured
- **Win condition**: Capture the opponent's King
- **Cooldown/Rest system**: After ANY piece moves or jumps, it enters a "rest" state and cannot perform ANY action until cooldown expires
  - Each piece type carries its own move cooldown in its `PieceDefinition`, so a new piece can be given a longer or shorter rest than the standard set.
  - **All six standard pieces are currently configured to the same move cooldown** — a pawn and a queen rest equally long. That is a deliberate balance decision for the standard set, not a placeholder: don't "fix" it by spreading the values apart. It is a change of numbers in the catalog, never a change of structure.
  - The jump cooldown is shared by every piece and is shorter than the move cooldown.
- **Jump command**: A piece can jump in place with a SHORT cooldown (used for defense)
  - Jump is a manual command only — the player must explicitly issue a "Jump" action for that piece. It is never triggered automatically as a dodge.
  - A piece is only protected from capture while it is actively mid-jump (i.e., during the jump's own short cooldown). Once that cooldown ends and the piece returns to its normal/idle state, it has no special protection — it is subject to the ordinary Collision Rules like any other piece.

## Collision Rules
- **Opposite-color collision**: When pieces of different colors end up on the same cell — regardless of whether one or both pieces moved to get there — the piece that arrived at the cell later captures the piece that was already there (arrived earlier).
- **Same-color near-collision**: When two pieces of the same color would occupy the same cell, the piece that arrives later stops one cell short of the collision point (in the cell before it) instead of completing its move; it does not land on or pass through the occupied cell.
  - Example: A Rook moves e1→e8 while a same-color Queen moves a4→h4. Their paths cross at e4. Since the Rook would "almost" collide with the Queen at e4, the Rook instead stops at e3.
- **Movement lock (mandatory for all rules above)**: A piece that is already mid-move cannot be commanded to start a new move to a different destination until its current move finishes.

## Piece Types
- Standard: Pawn, Rook, Knight, Bishop, Queen, King (standard chess movement rules)
- Optional future piece: Quadcopter — moves to any square in [±2, ±2] range, longer cooldown

## Scoring
Score = total cost of captured opponent pieces:
Pawn=1, Knight=3, Bishop=3, Rook=5, Queen=9
King=immediate win (no points)

## Game Lifecycle
- Game initializes with standard board setup
- Both players can move simultaneously
- Each move triggers cooldown
- Board updates in real-time
- When King is captured → Game Over, winner announced

## Functional Requirements
- Moves log: timestamp | player name | move (e.g., "e5" or "Jump" or "Nf3")
- Display player names
- Display current score per player
- Real-time board updates
- Detect when a King is captured → announce winner

## Non-Functional Requirements
- Scalable to thousands of simultaneous games — many games sharing **one** rule set, not many rule sets. There is a single `PieceCatalog::standard()` for the whole process; a game never needs its own catalog. Don't propose per-game or injectable catalogs.
- Business logic must be fully decoupled from UI
- Code must handle simultaneous moves (both players move at the same time)

## Architecture Constraints
### Binary Representation (future)
No binary implementation needed now, but code must support it easily.
All board/piece data must go through abstraction layers (interfaces/abstract classes).
Never access raw board data directly from outside the Board class.
**Design approach**: Use strategy pattern for piece representation — can swap text for binary later.

### Custom Game Rules
All piece movement rules must be data-driven or strategy-pattern based, and nothing about a piece may be hard-coded at its call sites.

**Current state**: every piece attribute — symbol, capture cost, move cooldown, promotion target, movement rule — lives in a `PieceDefinition` registered in `buildStandardCatalog()` (`src/logic/config/PieceCatalog.cpp`). Adding a piece is: one `PieceType` enum value, one `MovementRule` subclass, one `registerPiece` line. No switch statements over `PieceType` anywhere — reintroducing one is a regression.

**Still missing (future)**: the definitions are constructed in C++ rather than loaded from a config file, so end users cannot yet define their own pieces. The catalog is the seam where that loading will plug in; it should require no changes outside `PieceCatalog.cpp`.

**Exception**: the jump cooldown is a single process-wide constant in `src/logic/realtime/CooldownConfig.cpp`, not a per-piece field — see the Cooldown/Rest system rules above.

## Design Patterns
- Every component must be designed according to established design patterns (e.g. Strategy, Factory, Observer, Command, Decorator) rather than ad-hoc logic.
- Choose the pattern that fits the problem — don't force a pattern where plain code is clearer, but default to a well-known pattern when a class needs to vary behavior, create objects, notify listeners, or encapsulate an action.
- Reuse the patterns already established in this codebase (e.g. strategy pattern for piece representation and movement rules) before introducing a new one for similar problems.

## Code Quality Rules
- **DRY**: Every piece of logic exists in exactly one place
- **SRP**: Every function/class does ONE thing only
- **Encapsulation**: No class exposes internal data structures
- **No magic numbers or strings**: All constants live in config/enums
- **Clean Code**: Follow clean code cheat sheet standards
- **Minimal comments**: Write as few code comments as possible. Only add a comment when there is non-obvious/interesting logic that genuinely needs explaining — not to restate what the code already says.
- **Meaningful names**: Variable and function names must clearly express their purpose/intent.
- **Single responsibility per function**: Every function does exactly one thing (reinforces SRP above at the function level).

## Testing Requirements
- Aim for 100% unit test coverage
- Generate HTML coverage reports
- Framework is **doctest**, vendored at `tests/doctest/` so the build works offline. Do not introduce Google Test or any other framework.
- Test edge cases: simultaneous moves, cooldown interactions, king capture

## Source Control
- Commit after every meaningful addition
- Commit message format:
  - **Subject line**: `<Action verb> <what it was done to>: <short, professional description>`
    - Action verb first (e.g. Add, Fix, Refactor, Remove), then the feature/component it applies to, then a colon and a concise professional summary.
    - Example: `Add view/PieceView: per-piece sprite frame lookup by animation state`
  - **Body** (below the subject, after a blank line): a detailed, professional description of the change — but not too long (a few sentences).
