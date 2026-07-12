#pragma once
#include "MovementRule.h"

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
// toward row 0); captures one cell diagonally in that same direction.
class WhitePawnMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool isLegalCapture(int fromRow, int fromCol, int toRow, int toCol) const override;
};

// Moves one cell toward increasing row ("downward" on the printed board);
// captures one cell diagonally in that same direction.
class BlackPawnMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
    bool isLegalCapture(int fromRow, int fromCol, int toRow, int toCol) const override;
};
