#include "model/Board.h"
#include <cstdlib>

Board::Board(int rows, int cols)
    : rows_(rows), cols_(cols), grid_(static_cast<size_t>(rows) * cols) {
}

int Board::rowCount() const { return rows_; }
int Board::colCount() const { return cols_; }

bool Board::inBounds(int row, int col) const {
    return row >= 0 && row < rows_ && col >= 0 && col < cols_;
}

int Board::index(int row, int col) const {
    return row * cols_ + col;
}

bool Board::addPiece(int row, int col, Piece piece) {
    if (!inBounds(row, col)) return false;
    if (!isEmpty(row, col)) return false;
    grid_[index(row, col)] = std::move(piece);
    return true;
}

void Board::removePiece(int row, int col) {
    if (!inBounds(row, col)) return;
    grid_[index(row, col)].reset();
}

const Piece* Board::pieceAt(int row, int col) const {
    if (!inBounds(row, col)) return nullptr;
    const std::optional<Piece>& cell = grid_[index(row, col)];
    return cell.has_value() ? &(*cell) : nullptr;
}

// Assumes (row, col) is in bounds — callers only ever reach here after an
// inBounds check (directly, or by construction as with isPathClear's
// internal walk between two already-validated cells).
bool Board::isEmpty(int row, int col) const {
    return !grid_[index(row, col)].has_value();
}

bool Board::isSameColor(int row1, int col1, int row2, int col2) const {
    const Piece* a = pieceAt(row1, col1);
    const Piece* b = pieceAt(row2, col2);
    if (!a || !b) return false;
    return a->color() == b->color();
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

void Board::movePiece(int fromRow, int fromCol, int toRow, int toCol) {
    if (!inBounds(fromRow, fromCol) || !inBounds(toRow, toCol)) return;
    std::optional<Piece>& from = grid_[index(fromRow, fromCol)];
    std::optional<Piece>& to = grid_[index(toRow, toCol)];
    to = std::move(from);
    from.reset();
    if (to.has_value()) to->moveTo(Position{toRow, toCol});
}

void Board::promote(int row, int col, PieceType newKind) {
    if (!inBounds(row, col) || isEmpty(row, col)) return;
    const Piece& current = *pieceAt(row, col);
    grid_[index(row, col)] = Piece(current.id(), current.color(), newKind, current.cell());
}
