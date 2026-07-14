#pragma once
#include <vector>
#include <optional>
#include "model/Piece.h"
#include "model/PieceType.h"

// Owner of the logical arrangement of pieces on the board: its dimensions
// and what occupies each cell. Board knows what exists; it holds no chess
// movement rules and decides no move's legality (that's MovementRule/
// RuleEngine). It also knows nothing about text or binary encoding (that's
// the io layer) — see BoardTextFormat for reading/writing board text.
class Board {
public:
    explicit Board(int rows = 0, int cols = 0);

    int rowCount() const;
    int colCount() const;

    bool inBounds(int row, int col) const;

    bool addPiece(int row, int col, Piece piece);

    void removePiece(int row, int col);

    const Piece* pieceAt(int row, int col) const;
    Piece* pieceAt(int row, int col);
    std::optional<Piece> pieceCopyAt(int row, int col) const;

    bool isEmpty(int row, int col) const;
    bool isSameColor(int row1, int col1, int row2, int col2) const;
    bool isPathClear(int fromRow, int fromCol, int toRow, int toCol) const;

    void movePiece(int fromRow, int fromCol, int toRow, int toCol);

    void promote(int row, int col, PieceType newKind);

private:
    int index(int row, int col) const;

    int rows_;
    int cols_;
    std::vector<std::optional<Piece>> grid_;
};
