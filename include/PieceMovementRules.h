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
