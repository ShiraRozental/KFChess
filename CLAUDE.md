# Chess Game Project (Modified Rules)

## Project Overview
A C++ chess game with modified rules, built iteratively.
Core business logic is in place; the project has now moved on to designing the rendering/view layer (see `include/view/`, e.g. `Renderer.h`).

## Modified Game Rules
- **Simultaneous play**: Both Black and White move at the same time, no turns
- **No Check/Checkmate**: Game ends ONLY when a King is captured
- **Win condition**: Capture the opponent's King
- **Cooldown/Rest system**: After ANY piece moves or jumps, it enters a "rest" state and cannot perform ANY action until cooldown expires
  - Each piece type has a different cooldown duration (configured)
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
- Scalable to thousands of simultaneous games
- Business logic must be fully decoupled from UI
- Code must handle simultaneous moves (both players move at the same time)

## Architecture Constraints
### Binary Representation (future)
No binary implementation needed now, but code must support it easily.
All board/piece data must go through abstraction layers (interfaces/abstract classes).
Never access raw board data directly from outside the Board class.
**Design approach**: Use strategy pattern for piece representation — can swap text for binary later.

### Custom Game Rules (future)
Nothing must be hard-coded.
All piece movement rules must be data-driven or strategy-pattern based.
Example: pawn promotion behavior must be injectable, not hard-coded.
Users should eventually be able to define their own pieces and movement rules.
All cooldown durations must come from configuration, not code.
**Design approach**: PieceDefinition objects loaded from config, not hard-coded in code.

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
- Use a proper C++ testing framework (e.g. Google Test)
- Test edge cases: simultaneous moves, cooldown interactions, king capture

## Source Control
- Commit after every meaningful addition
- Commit message format: 'Add [feature/component]: [description]'
