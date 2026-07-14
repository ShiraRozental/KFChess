#include "engine/GameEngine.h"
#include "io/BoardTextFormat.h"
#include "model/Piece.h"
#include "rules/RuleEngine.h"
#include "rules/MovementRuleFactory.h"
#include <sstream>
#include <utility>

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

    // A mid-jump piece is airborne — off the board entirely — so validation
    // below would report empty_source; the truthful reason is that the
    // piece is mid-motion and cannot be commanded until it lands.
    if (arbiter_.hasJumpAt(source)) return MoveResult{false, MoveResultReason::MotionInProgress};

    // A friend jumping over the destination is also off the board, so
    // RuleEngine can't reject the move itself — restore the same
    // friendly_destination answer it would have given were the jumper home.
    const Piece* sourcePiece = board_.pieceAt(source.row, source.col);
    if (sourcePiece && arbiter_.jumpColorAt(destination) == std::optional<PieceColor>(sourcePiece->color())) {
        return MoveResult{false, MoveRejectionReason::FriendlyDestination};
    }

    MoveValidation validation = validateMove(board_, source.row, source.col, destination.row, destination.col);
    if (!validation.is_valid) return MoveResult{false, validation.reason};

    // Guaranteed non-null: validateMove already confirmed a piece sits here.
    // The mover leaves the board for the duration of its flight; Motion owns
    // it until an ArrivalEvent hands it back (see RealTimeArbiter).
    Piece mover = *sourcePiece;
    board_.removePiece(source.row, source.col);
    arbiter_.startMotion(std::move(mover), source, destination);
    return MoveResult{true, MoveResultReason::Ok};
}

// Jump is outside the course's minimal API (CLAUDE.md requires it as its
// own explicit action) but reuses the same underlying motion mechanism: a
// jump is just a motion whose source equals its destination.
void GameEngine::requestJump(const Position& cell) {
    if (isGameOver()) return;

    // An empty cell also covers a piece that is already airborne (moving or
    // jumping) — it is off the board, so there is nothing to command.
    const Piece* piece = board_.pieceAt(cell.row, cell.col);
    if (!piece) return;

    Piece jumper = *piece;
    board_.removePiece(cell.row, cell.col);
    arbiter_.startMotion(std::move(jumper), cell, cell);
}

bool GameEngine::hasPieceAt(const Position& pos) const {
    return board_.pieceAt(pos.row, pos.col) != nullptr;
}

// Advances simulated time and applies every motion RealTimeArbiter reports
// as arrived. An intercepted attacker never lands: its piece is simply
// discarded, leaving the defender (whose jump protected the cell)
// untouched. If a captured piece is a king, the game ends immediately;
// motions still due in this same batch are voided and their pieces return
// to where they took off from, so no piece ever vanishes from the board.
void GameEngine::wait(int ms) {
    if (isGameOver()) return;

    for (const ArrivalEvent& event : arbiter_.advanceTime(ms)) {
        if (event.intercepted) {
            if (event.piece.kind() == PieceType::King) {
                gameState_ = winningStateFor(opponentOf(event.piece.color()));
            }
            continue;
        }

        if (isGameOver()) {
            landPiece(event.piece, event.from);
            continue;
        }

        const Piece* target = board_.pieceAt(event.to.row, event.to.col);
        bool capturesKing = target && target->kind() == PieceType::King;

        board_.removePiece(event.to.row, event.to.col);
        landPiece(event.piece, event.to);

        if (capturesKing) {
            gameState_ = winningStateFor(event.piece.color());
        }

        promoteIfNeeded(event.to.row, event.to.col, event.piece.kind(), event.piece.color());
    }
}

// The snapshot's board is a display composition: the settled board plus
// every in-flight piece shown at its current cell, so a piece mid-motion
// never "disappears" from a printed board or a renderer.
GameSnapshot GameEngine::snapshot() const {
    Board display = board_;
    for (const InFlightPiece& flier : arbiter_.inFlightPieces()) {
        display.addPiece(flier.cell.row, flier.cell.col, flier.piece);
    }
    return GameSnapshot(std::move(display), isGameOver(), winner());
}

// Places a piece whose flight has resolved (normally, or voided by the game
// ending mid-batch) back onto the board at the given cell.
void GameEngine::landPiece(Piece piece, Position cell) {
    piece.moveTo(cell);
    board_.addPiece(cell.row, cell.col, std::move(piece));
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

PieceColor GameEngine::opponentOf(PieceColor color) {
    return color == PieceColor::White ? PieceColor::Black : PieceColor::White;
}
