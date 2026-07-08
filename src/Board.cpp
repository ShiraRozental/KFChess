#include "Board.h"
#include <sstream>
namespace {
    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
}

// Returns true if the token has a valid chess-piece format.
bool Board::isValidToken(const std::string& token) {
    if (token == ".") return true;
    if (token.length() != 2) return false;
    char color = token[0];
    char piece = token[1];
    if (color != 'w' && color != 'b') return false;
    return std::string("KQRBNP").find(piece) != std::string::npos;
}

// Reads the board from an input stream and stores it in the board object.
bool Board::fromStream(std::istream& in, Board& outBoard, std::string& errorMessage) {
    std::string line;
    bool readingBoard = false;

    while (std::getline(in, line)) {
        std::string marker = trim(line);
        if (marker == "Board:") { readingBoard = true; continue; }
        if (marker == "Commands:") { readingBoard = false; continue; }

        if (!readingBoard || line.empty()) continue;

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

        outBoard.rows_.push_back(line);
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

// Returns the token stored at the given board position.
const std::string& Board::tokenAt(int row, int col) const {
    return grid_[row][col];
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
    std::string rebuilt;
    for (size_t c = 0; c < grid_[row].size(); ++c) {
        if (c > 0) rebuilt += " ";
        rebuilt += grid_[row][c];
    }
    rows_[row] = rebuilt;
}