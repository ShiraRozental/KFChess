#include "engine/GameEngine.h"
#include "io/BoardTextFormat.h"
#include "model/Piece.h"
#include "rules/RuleEngine.h"
#include "rules/MovementRuleFactory.h"
#include <sstream>

// Loads a board description into the game object.
bool GameEngine::loadBoard(const std::string& boardText, std::string& errorMessage) {
    std::istringstream in(boardText);
    return BoardTextFormat::parse(in, board_, errorMessage);
}

// Application-level guards (game_over, motion_in_progress) are checked
// before RuleEngine is ever consulted; RuleEngine knows nothing about
// either. A rule-level rejection's reason is copied straight through.
MoveResult GameEngine::requestMove(const Position& source, const Position& destination) {
    if (isGameOver()) return MoveResult{false, MoveResultReason::GameOver};
    if (arbiter_.hasActiveMotion()) return MoveResult{false, MoveResultReason::MotionInProgress};

    MoveValidation validation = validateMove(board_, source.row, source.col, destination.row, destination.col);
    if (!validation.is_valid) return MoveResult{false, validation.reason};

    // Guaranteed non-null: validateMove already confirmed a piece sits here.
    Piece* mover = board_.pieceAt(source.row, source.col);

    // hasActiveMotion() only reports real moves (see RealTimeArbiter), so a
    // piece that's currently jumping wouldn't otherwise be caught here.
    if (mover->state() == Piece::State::Moving) {
        return MoveResult{false, MoveResultReason::MotionInProgress};
    }

    arbiter_.startMotion(*mover, source, destination);
    return MoveResult{true, MoveResultReason::Ok};
}

// Jump is outside the course's minimal API (CLAUDE.md requires it as its
// own explicit action) but reuses the same underlying motion mechanism: a
// jump is just a motion whose source equals its destination.
void GameEngine::requestJump(const Position& cell) {
    if (isGameOver()) return;

    Piece* piece = board_.pieceAt(cell.row, cell.col);
    if (!piece) return;
    if (piece->state() == Piece::State::Moving) return;

    arbiter_.startMotion(*piece, cell, cell);
}

bool GameEngine::hasPieceAt(const Position& pos) const {
    return board_.pieceAt(pos.row, pos.col) != nullptr;
}

// Advances simulated time and applies every motion RealTimeArbiter reports
// as arrived. An intercepted attacker never lands: it's simply removed
// from its origin, leaving the defender (whose jump protected the cell)
// untouched. If a captured piece is a king, the game ends immediately and
// any further due arrivals in this same batch are left unapplied.
void GameEngine::wait(int ms) {
    if (isGameOver()) return;

    for (const ArrivalEvent& event : arbiter_.advanceTime(ms)) {
        if (isGameOver()) break;

        if (event.intercepted) {
            board_.removePiece(event.from.row, event.from.col);
            continue;
        }

        const Piece* mover = board_.pieceAt(event.from.row, event.from.col);
        const Piece* target = board_.pieceAt(event.to.row, event.to.col);
        bool capturesKing = target && target->kind() == PieceType::King;
        PieceColor moverColor = mover->color();
        PieceType movedType = mover->kind();

        board_.movePiece(event.from.row, event.from.col, event.to.row, event.to.col);

        if (capturesKing) {
            gameState_ = winningStateFor(moverColor);
        }

        promoteIfNeeded(event.to.row, event.to.col, movedType, moverColor);
    }
}

GameSnapshot GameEngine::snapshot() const {
    return GameSnapshot(board_, isGameOver(), winner());
}

// Promotes the piece now sitting at (row, col) to a queen if its movement
// rule reports that this row is far enough to promote (only pawns do).
// Piece type and color must be the ones the piece had before this move, so
// promotion is decided by the rule that actually made the move, not by
// whatever ends up on the board.
void GameEngine::promoteIfNeeded(int row, int col, PieceType type, PieceColor color) {
    const MovementRule& rule = movementRuleFor(type, color);
    if (rule.reachesPromotionRow(row, board_.rowCount())) {
        board_.promote(row, col, PieceType::Queen);
    }
}

// Returns true once a king has been captured and the game has a winner.
bool GameEngine::isGameOver() const {
    return gameState_ != GameState::InProgress;
}

// Returns the winning color, or nullopt while the game is still in progress.
std::optional<PieceColor> GameEngine::winner() const {
    if (gameState_ == GameState::WhiteWins) return PieceColor::White;
    if (gameState_ == GameState::BlackWins) return PieceColor::Black;
    return std::nullopt;
}

// Maps the color of the piece that captured the enemy king to the
// corresponding game outcome.
GameState GameEngine::winningStateFor(PieceColor color) {
    return color == PieceColor::White ? GameState::WhiteWins : GameState::BlackWins;
}
