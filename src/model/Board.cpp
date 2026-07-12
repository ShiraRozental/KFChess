#include "model/Board.h"
#include "io/StringUtils.h"
#include <cstdlib>
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

    // Maps a token's piece letter to its PieceType. Returns nullopt for an
    // unrecognized letter (should not happen for tokens that already passed
    // isValidToken).
    std::optional<PieceType> charToPieceType(char c) {
        switch (c) {
            case 'K': return PieceType::King;
            case 'Q': return PieceType::Queen;
            case 'R': return PieceType::Rook;
            case 'B': return PieceType::Bishop;
            case 'N': return PieceType::Knight;
            case 'P': return PieceType::Pawn;
            default:  return std::nullopt;
        }
    }

    // Maps a token's color letter to its PieceColor. Returns nullopt for an
    // unrecognized letter (should not happen for tokens that already passed
    // isValidToken).
    std::optional<PieceColor> charToPieceColor(char c) {
        switch (c) {
            case kWhiteColorSymbol: return PieceColor::White;
            case kBlackColorSymbol: return PieceColor::Black;
            default:                return std::nullopt;
        }
    }

    // Maps a PieceType to its token letter. Inverse of charToPieceType.
    char pieceTypeToChar(PieceType type) {
        switch (type) {
            case PieceType::King:   return 'K';
            case PieceType::Queen:  return 'Q';
            case PieceType::Rook:   return 'R';
            case PieceType::Bishop: return 'B';
            case PieceType::Knight: return 'N';
            case PieceType::Pawn:   return 'P';
        }
        return 'Q'; // unreachable for valid enum values; keeps all paths returning
    }
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

// Checks whether every cell strictly between two straight- or diagonally-
// aligned cells is empty. Used to block sliding pieces (rook/bishop/queen)
// from moving through other pieces. For deltas that are neither straight nor
// diagonal (e.g. a knight's shape), there is no well-defined path to check,
// so this returns true vacuously — callers only invoke it after the shape
// has already been validated as straight/diagonal.
bool Board::isPathClear(int fromRow, int fromCol, int toRow, int toCol) const {
    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;
    bool isStraightOrDiagonal = (dRow == 0 || dCol == 0 || std::abs(dRow) == std::abs(dCol));
    if (!isStraightOrDiagonal) return true;

    int stepRow = (dRow > 0) - (dRow < 0);
    int stepCol = (dCol > 0) - (dCol < 0);

    int row = fromRow + stepRow;
    int col = fromCol + stepCol;
    while (row != toRow || col != toCol) {
        if (!isEmpty(row, col)) return false;
        row += stepRow;
        col += stepCol;
    }
    return true;
}

// Moves a piece from one cell to another and rebuilds the affected rows.
void Board::movePiece(int fromRow, int fromCol, int toRow, int toCol) {
    if (!inBounds(fromRow, fromCol) || !inBounds(toRow, toCol)) return;
    grid_[toRow][toCol] = grid_[fromRow][fromCol];
    grid_[fromRow][fromCol] = ".";
    rebuildRow(fromRow);
    rebuildRow(toRow);
}

// Removes the piece at a cell, leaving it empty. Unlike movePiece, this does
// not touch any other cell — used when a piece is captured mid-transit
// without ever reaching its destination (e.g. an airborne jump defender).
void Board::removePiece(int row, int col) {
    if (!inBounds(row, col)) return;
    grid_[row][col] = ".";
    rebuildRow(row);
}

// Rebuilds the text row from the current grid values.
void Board::rebuildRow(int row) {
    rows_[row] = joinTokens(grid_[row]);
}

// Changes the piece type at a cell while preserving its color (e.g. pawn
// promotion). Does nothing for an out-of-bounds or empty cell.
void Board::setPieceType(int row, int col, PieceType type) {
    if (!inBounds(row, col) || isEmpty(row, col)) return;
    grid_[row][col][1] = pieceTypeToChar(type);
    rebuildRow(row);
}

// Returns the number of rows in the parsed board.
int Board::rowCount() const {
    return (int)grid_.size();
}

// Returns the piece type at a cell, or nullopt if the cell is out of bounds
// or empty.
std::optional<PieceType> Board::pieceTypeAt(int row, int col) const {
    if (!inBounds(row, col) || isEmpty(row, col)) return std::nullopt;
    return charToPieceType(grid_[row][col][1]);
}

// Returns the piece color at a cell, or nullopt if the cell is out of bounds
// or empty.
std::optional<PieceColor> Board::colorAt(int row, int col) const {
    if (!inBounds(row, col) || isEmpty(row, col)) return std::nullopt;
    return charToPieceColor(grid_[row][col][0]);
}