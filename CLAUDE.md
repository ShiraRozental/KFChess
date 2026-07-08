# Chess Game Project (Modified Rules)

## Project Overview
A C++ chess game with modified rules, built iteratively.
Business logic only — no graphics yet.

## Modified Game Rules
- **Simultaneous play**: Both Black and White move at the same time, no turns
- **No Check/Checkmate**: Game ends ONLY when a King is captured
- **Win condition**: Capture the opponent's King
- **Cooldown/Rest system**: After ANY piece moves or jumps, it enters a "rest" state and cannot perform ANY action until cooldown expires
  - Each piece type has a different cooldown duration (configured)
- **Jump command**: A piece can jump in place with a SHORT cooldown (used for defense)
  - Auto-dodge: if an opponent tries to capture a piece that is still mid-cooldown from its last action, the piece automatically jumps in place, ending its cooldown early and avoiding the capture
  - If the piece's cooldown has already finished, it has no protection and can be captured normally

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

## Code Quality Rules
- **DRY**: Every piece of logic exists in exactly one place
- **SRP**: Every function/class does ONE thing only
- **Encapsulation**: No class exposes internal data structures
- **No magic numbers or strings**: All constants live in config/enums
- **Clean Code**: Follow clean code cheat sheet standards

## Testing Requirements
- Aim for 100% unit test coverage
- Generate HTML coverage reports
- Use a proper C++ testing framework (e.g. Google Test)
- Test edge cases: simultaneous moves, cooldown interactions, king capture

## Source Control
- Commit after every meaningful addition
- Commit message format: 'Add [feature/component]: [description]'
