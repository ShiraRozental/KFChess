#pragma once
#include "logic/rules/MovementRule.h"

class KingMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
};

class QueenMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool requiresClearPath() const override { return true; }
};

class RookMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool requiresClearPath() const override { return true; }
};

class BishopMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool requiresClearPath() const override { return true; }
};

class KnightMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
};

// Moves one cell toward decreasing row ("upward" on the printed board, i.e.
// toward row 0); captures one cell diagonally in that same direction. From
// its home row (the bottom edge, its own back row) it may also advance two
// cells in one move. Promotes on reaching row 0, the far edge.
class WhitePawnMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool isLegalCapture(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool isLegalDoubleMove(int fromRow, int fromCol, int toRow, int toCol, int boardHeight) const override;
    bool reachesPromotionRow(int row, int boardHeight) const override;
};

// Moves one cell toward increasing row ("downward" on the printed board);
// captures one cell diagonally in that same direction. From its home row
// (row 0, the top edge, its own back row) it may also advance two cells in
// one move. Promotes on reaching the last row, the far edge.
class BlackPawnMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool isLegalCapture(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool isLegalDoubleMove(int fromRow, int fromCol, int toRow, int toCol, int boardHeight) const override;
    bool reachesPromotionRow(int row, int boardHeight) const override;
};
