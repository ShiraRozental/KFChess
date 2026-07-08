#include "Board.h"
#include "StringUtils.h"
#include <sstream>
namespace {
    // Joins tokens with a single space to produce the canonical row text.
    std::string joinTokens(const std::vector<std::string>& tokens) {
        std::string joined;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) joined += " ";
            joined += tokens[i];
        }
        return joined;
    }

    const char kEmptyCellSymbol = '.';
    const char kWhiteColorSymbol = 'w';
    const char kBlackColorSymbol = 'b';
    const std::string kValidPieceLetters = "KQRBNP";
}

// Returns true if the token has a valid chess-piece format.
bool Board::isValidToken(const std::string& token) {
    if (token.length() == 1 && token[0] == kEmptyCellSymbol) return true;
    if (token.length() != 2) return false;
    char color = token[0];
    char piece = token[1];
    if (color != kWhiteColorSymbol && color != kBlackColorSymbol) return false;
    return kValidPieceLetters.find(piece) != std::string::npos;
}

// Reads the board from an input stream and stores it in the board object.
bool Board::fromStream(std::istream& in, Board& outBoard, std::string& errorMessage) {
    std::string line;
    bool readingBoard = false;

    while (std::getline(in, line)) {
        std::string marker = trim(line);
        if (marker == "Board:") { readingBoard = true; continue; }
        if (marker == "Commands:") { readingBoard = false; continue; }

        if (!readingBoard || marker.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (ss >> token) {
            if (!isValidToken(token)) {
                errorMessage = "ERROR UNKNOWN_TOKEN";
                return false;
            }
            tokens.push_back(token);
        }

        int currentWidth = (int)tokens.size();
        if (outBoard.width_ == -1) {
            outBoard.width_ = currentWidth;
        } else if (currentWidth != outBoard.width_) {
            errorMessage = "ERROR ROW_WIDTH_MISMATCH";
            return false;
        }

        outBoard.rows_.push_back(joinTokens(tokens));
        outBoard.grid_.push_back(tokens);
    }
    return true;
}

// Prints the current board text to the given output stream.
void Board::print(std::ostream& out) const {
    for (size_t i = 0; i < rows_.size(); ++i) {
        out << rows_[i];
        if (i < rows_.size() - 1) out << "\n";
    }
}

// Checks whether a row and column are inside the board bounds.
bool Board::inBounds(int row, int col) const {
    return row >= 0 && row < (int)grid_.size() && col >= 0 && col < width_;
}

// Checks whether a cell has no piece on it.
bool Board::isEmpty(int row, int col) const {
    return grid_[row][col][0] == kEmptyCellSymbol;
}

// Checks whether two cells both hold pieces of the same color.
bool Board::isSameColor(int row1, int col1, int row2, int col2) const {
    if (isEmpty(row1, col1) || isEmpty(row2, col2)) return false;
    return grid_[row1][col1][0] == grid_[row2][col2][0];
}

// Moves a piece from one cell to another and rebuilds the affected rows.
void Board::movePiece(int fromRow, int fromCol, int toRow, int toCol) {
    if (!inBounds(fromRow, fromCol) || !inBounds(toRow, toCol)) return;
    grid_[toRow][toCol] = grid_[fromRow][fromCol];
    grid_[fromRow][fromCol] = ".";
    rebuildRow(fromRow);
    rebuildRow(toRow);
}

// Rebuilds the text row from the current grid values.
void Board::rebuildRow(int row) {
    rows_[row] = joinTokens(grid_[row]);
}