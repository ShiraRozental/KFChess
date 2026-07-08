#pragma once
#include "MovementRule.h"

class KingMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
};

class QueenMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
};

class RookMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
};

class BishopMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
};

class KnightMovementRule : public MovementRule {
public:
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const override;
};
