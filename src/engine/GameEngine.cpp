#include "engine/GameEngine.h"
#include "io/BoardTextFormat.h"
#include "model/Piece.h"
#include "rules/RuleEngine.h"
#include "rules/MovementRuleFactory.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace {
    constexpr int kCellSizePixels = 100;
    constexpr long long kMoveDurationPerCellMs = 1000;
    constexpr long long kJumpDurationMs = 1000;

    // Converts a pixel coordinate to a cell index using floor division, so
    // negative pixels (left/above the board) map to negative cell indices
    // instead of wrapping to 0 under C++'s truncating integer division.
    int pixelToCell(int pixel) {
        if (pixel >= 0) return pixel / kCellSizePixels;
        return (pixel - (kCellSizePixels - 1)) / kCellSizePixels;
    }

    // Chebyshev distance (the standard "king move count" metric): straight
    // and diagonal steps both count as 1 cell of travel, matching how every
    // piece's shape is defined in terms of row/column deltas.
    int cellDistance(int fromRow, int fromCol, int toRow, int toCol) {
        return std::max(std::abs(toRow - fromRow), std::abs(toCol - fromCol));
    }
}

// Loads a board description into the game object.
bool GameEngine::loadBoard(const std::string& boardText, std::string& errorMessage) {
    std::istringstream in(boardText);
    return BoardTextFormat::parse(in, board_, errorMessage);
}

// Executes a single command line based on its keyword.
void GameEngine::executeLine(const std::string& line, std::ostream& out) {
    std::istringstream ss(line);
    std::string keyword;
    ss >> keyword;

    if (keyword == "click") {
        int x, y;
        if (ss >> x >> y) handleClick(x, y);
    } else if (keyword == "wait") {
        int ms;
        if (ss >> ms) handleWait(ms);
    } else if (keyword == "jump") {
        int x, y;
        if (ss >> x >> y) handleJump(x, y);
    } else if (keyword == "print") {
        std::string second;
        ss >> second;
        if (second == "board") handlePrintBoard(out);
    }
    // unknown keyword: ignored
}

// Handles a click by selecting or moving a piece based on the clicked cell.
void GameEngine::handleClick(int pixelX, int pixelY) {
    if (isGameOver()) return;

    int col = pixelToCell(pixelX);
    int row = pixelToCell(pixelY);

    if (!board_.inBounds(row, col)) return;

    bool cellHasPiece = !board_.isEmpty(row, col);

    if (!selected_.has_value()) {
        if (cellHasPiece) selected_ = Position{row, col};
        return;
    }

    if (cellHasPiece && board_.isSameColor(row, col, selected_->row, selected_->col)) {
        selected_ = Position{row, col};
        return;
    }

    if (!isAnyMovePending() &&
        validateMove(board_, selected_->row, selected_->col, row, col).legal) {
        // The board is not mutated yet: the move only takes effect once its
        // arrival time is reached (see applyDueMoves). Duration scales with
        // distance so a longer move takes proportionally longer to arrive.
        long long duration = cellDistance(selected_->row, selected_->col, row, col)
            * kMoveDurationPerCellMs;
        pendingMoves_.push_back(PendingMove{
            selected_->row, selected_->col, row, col, clockMs_ + duration});
    }
    selected_.reset();
}

// Adds waiting time to the game clock, then commits any pending moves whose
// arrival time has now been reached. Jumps are resolved after moves so that
// a jump landing in this same tick still counts as airborne for any move
// that arrives at its cell at the same moment (defense wins ties).
void GameEngine::handleWait(int ms) {
    if (isGameOver()) return;

    clockMs_ += ms;
    applyDueMoves();
    applyDueJumps();
}

// Handles a jump command by making the piece at the clicked cell jump in
// place. A piece that is mid-transit (moving) or already airborne cannot
// jump; both cases are silently ignored, matching how illegal clicks are
// ignored elsewhere.
void GameEngine::handleJump(int pixelX, int pixelY) {
    if (isGameOver()) return;

    int col = pixelToCell(pixelX);
    int row = pixelToCell(pixelY);

    if (!board_.inBounds(row, col)) return;
    if (board_.isEmpty(row, col)) return;
    if (isPieceMoving(row, col)) return;
    if (isPieceAirborne(row, col)) return;

    pendingJumps_.push_back(PendingJump{row, col, clockMs_ + kJumpDurationMs});
}

// Applies every pending move whose arrival time has passed to the board,
// and removes it from the pending list. Moves that have not yet arrived
// are left in place for a later wait to pick up. If a move captures a
// king, the game ends immediately and any remaining pending moves are
// left untouched (queued, never applied).
//
// If the destination cell holds a piece that is currently airborne (mid-
// jump), the arriving piece is captured instead: it never lands, so it is
// simply removed from its origin and the airborne defender is left exactly
// as it was (see PendingJump).
void GameEngine::applyDueMoves() {
    std::vector<PendingMove> stillPending;
    for (const PendingMove& move : pendingMoves_) {
        if (isGameOver()) {
            stillPending.push_back(move);
            continue;
        }
        if (move.arrivalTimeMs > clockMs_) {
            stillPending.push_back(move);
            continue;
        }

        if (isPieceAirborne(move.toRow, move.toCol)) {
            board_.removePiece(move.fromRow, move.fromCol);
            continue;
        }

        // Guaranteed non-null: a pending move is only ever scheduled for a
        // cell that held a piece (see handleClick), and only one move may
        // be in flight at a time (see isAnyMovePending), so nothing else
        // can have vacated this cell before it arrives here.
        const Piece* mover = board_.pieceAt(move.fromRow, move.fromCol);
        const Piece* target = board_.pieceAt(move.toRow, move.toCol);
        bool capturesKing = target && target->kind() == PieceType::King;
        PieceColor moverColor = mover->color();
        PieceType movedType = mover->kind();

        board_.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);

        if (capturesKing) {
            gameState_ = winningStateFor(moverColor);
        }

        promoteIfNeeded(move.toRow, move.toCol, movedType, moverColor);
    }
    pendingMoves_ = std::move(stillPending);
}

// Removes every jump whose landing time has passed. A landed jump needs no
// board mutation: the piece never left its cell.
void GameEngine::applyDueJumps() {
    std::vector<PendingJump> stillAirborne;
    for (const PendingJump& jump : pendingJumps_) {
        if (jump.endTimeMs > clockMs_) {
            stillAirborne.push_back(jump);
        }
    }
    pendingJumps_ = std::move(stillAirborne);
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

// Returns true if some piece (of either color) is currently mid-transit
// toward a pending destination. Only one move may be in flight on the board
// at a time: no new move can be scheduled until the current one arrives.
bool GameEngine::isAnyMovePending() const {
    return !pendingMoves_.empty();
}

// Returns true if the piece at (row, col) is currently mid-transit as the
// origin of some pending move (a moving piece cannot also jump).
bool GameEngine::isPieceMoving(int row, int col) const {
    for (const PendingMove& move : pendingMoves_) {
        if (move.fromRow == row && move.fromCol == col) return true;
    }
    return false;
}

// Returns true if the piece at (row, col) is currently airborne from a
// jump that has not yet landed.
bool GameEngine::isPieceAirborne(int row, int col) const {
    for (const PendingJump& jump : pendingJumps_) {
        if (jump.row == row && jump.col == col) return true;
    }
    return false;
}

// Prints the current board state to the output stream.
void GameEngine::handlePrintBoard(std::ostream& out) {
    BoardTextFormat::write(board_, out);
    out << "\n";
}