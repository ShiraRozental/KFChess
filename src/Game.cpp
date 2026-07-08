#include "Game.h"
#include <sstream>

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
        ss >> x >> y;
        handleClick(x, y);
    } else if (keyword == "wait") {
        int ms;
        ss >> ms;
        handleWait(ms);
    } else if (keyword == "print") {
        std::string second;
        ss >> second;
        if (second == "board") handlePrintBoard(out);
    }
    // unknown keyword: ignored
}

// Handles a click by selecting or moving a piece based on the clicked cell.
void Game::handleClick(int pixelX, int pixelY) {
    int col = pixelX / 100;
    int row = pixelY / 100;

    if (!board_.inBounds(row, col)) return;

    const std::string& token = board_.tokenAt(row, col);
    bool cellHasPiece = (token != ".");

    if (!selected_.has_value()) {
        if (cellHasPiece) selected_ = Position{row, col};
        return;
    }

    const std::string& selectedToken = board_.tokenAt(selected_->row, selected_->col);
    if (cellHasPiece && token[0] == selectedToken[0]) {
        selected_ = Position{row, col};
        return;
    }

    board_.movePiece(selected_->row, selected_->col, row, col);
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