#include "Game.h"
#include "MoveLegality.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace {
    constexpr int kCellSizePixels = 100;
    constexpr long long kMoveDurationPerCellMs = 1000;

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
bool Game::loadBoard(const std::string& boardText, std::string& errorMessage) {
    std::istringstream in(boardText);
    return Board::fromStream(in, board_, errorMessage);
}

// Executes a single command line based on its keyword.
void Game::executeLine(const std::string& line, std::ostream& out) {
    std::istringstream ss(line);
    std::string keyword;
    ss >> keyword;

    if (keyword == "click") {
        int x, y;
        if (ss >> x >> y) handleClick(x, y);
    } else if (keyword == "wait") {
        int ms;
        if (ss >> ms) handleWait(ms);
    } else if (keyword == "print") {
        std::string second;
        ss >> second;
        if (second == "board") handlePrintBoard(out);
    }
    // unknown keyword: ignored
}

// Handles a click by selecting or moving a piece based on the clicked cell.
void Game::handleClick(int pixelX, int pixelY) {
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

    std::optional<PieceType> movingType = board_.pieceTypeAt(selected_->row, selected_->col);
    if (movingType.has_value() &&
        !isAnyMovePending() &&
        isLegalMove(board_, *movingType, selected_->row, selected_->col, row, col)) {
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
// arrival time has now been reached.
void Game::handleWait(int ms) {
    if (isGameOver()) return;

    clockMs_ += ms;
    applyDueMoves();
}

// Applies every pending move whose arrival time has passed to the board,
// and removes it from the pending list. Moves that have not yet arrived
// are left in place for a later wait to pick up. If a move captures a
// king, the game ends immediately and any remaining pending moves are
// left untouched (queued, never applied).
void Game::applyDueMoves() {
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

        std::optional<PieceColor> moverColor = board_.colorAt(move.fromRow, move.fromCol);
        bool capturesKing = board_.pieceTypeAt(move.toRow, move.toCol) == PieceType::King;

        board_.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);

        if (capturesKing && moverColor.has_value()) {
            gameState_ = winningStateFor(*moverColor);
        }
    }
    pendingMoves_ = std::move(stillPending);
}

// Returns true once a king has been captured and the game has a winner.
bool Game::isGameOver() const {
    return gameState_ != GameState::InProgress;
}

// Returns the winning color, or nullopt while the game is still in progress.
std::optional<PieceColor> Game::winner() const {
    if (gameState_ == GameState::WhiteWins) return PieceColor::White;
    if (gameState_ == GameState::BlackWins) return PieceColor::Black;
    return std::nullopt;
}

// Maps the color of the piece that captured the enemy king to the
// corresponding game outcome.
GameState Game::winningStateFor(PieceColor color) {
    return color == PieceColor::White ? GameState::WhiteWins : GameState::BlackWins;
}

// Returns true if some piece (of either color) is currently mid-transit
// toward a pending destination. Only one move may be in flight on the board
// at a time: no new move can be scheduled until the current one arrives.
bool Game::isAnyMovePending() const {
    return !pendingMoves_.empty();
}

// Prints the current board state to the output stream.
void Game::handlePrintBoard(std::ostream& out) {
    board_.print(out);
    out << "\n";
}