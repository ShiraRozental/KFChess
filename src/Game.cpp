#include "Game.h"
#include "MoveLegality.h"
#include <sstream>

namespace {
    constexpr int kCellSizePixels = 100;

    // Converts a pixel coordinate to a cell index using floor division, so
    // negative pixels (left/above the board) map to negative cell indices
    // instead of wrapping to 0 under C++'s truncating integer division.
    int pixelToCell(int pixel) {
        if (pixel >= 0) return pixel / kCellSizePixels;
        return (pixel - (kCellSizePixels - 1)) / kCellSizePixels;
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
        isLegalMove(board_, *movingType, selected_->row, selected_->col, row, col)) {
        board_.movePiece(selected_->row, selected_->col, row, col);
    }
    selected_.reset();
}

// Adds waiting time to the game clock.
void Game::handleWait(int ms) {
    clockMs_ += ms;
    // No delayed/pending moves yet — moves resolve instantly until
    // Cooldown exists. Hook point for later.
}

// Prints the current board state to the output stream.
void Game::handlePrintBoard(std::ostream& out) {
    board_.print(out);
    out << "\n";
}