#include "PieceMovementRules.h"
#include <cstdlib>

namespace {
    bool isZeroMove(int dRow, int dCol) {
        return dRow == 0 && dCol == 0;
    }

    // Straight horizontal or vertical line, excluding the zero-move case.
    bool isRookShape(int dRow, int dCol) {
        return (dRow == 0) != (dCol == 0);
    }

    // Diagonal line, excluding the zero-move case.
    bool isBishopShape(int dRow, int dCol) {
        return !isZeroMove(dRow, dCol) && std::abs(dRow) == std::abs(dCol);
    }
}

bool KingMovementRule::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const {
    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;
    return !isZeroMove(dRow, dCol) && std::abs(dRow) <= 1 && std::abs(dCol) <= 1;
}

bool RookMovementRule::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const {
    return isRookShape(toRow - fromRow, toCol - fromCol);
}

bool BishopMovementRule::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const {
    return isBishopShape(toRow - fromRow, toCol - fromCol);
}

bool QueenMovementRule::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const {
    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;
    return isRookShape(dRow, dCol) || isBishopShape(dRow, dCol);
}

bool KnightMovementRule::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const {
    int dRow = std::abs(toRow - fromRow);
    int dCol = std::abs(toCol - fromCol);
    return (dRow == 1 && dCol == 2) || (dRow == 2 && dCol == 1);
}

bool WhitePawnMovementRule::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const {
    return toCol == fromCol && toRow == fromRow - 1;
}

bool WhitePawnMovementRule::isLegalCapture(int fromRow, int fromCol, int toRow, int toCol) const {
    return std::abs(toCol - fromCol) == 1 && toRow == fromRow - 1;
}

bool BlackPawnMovementRule::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const {
    return toCol == fromCol && toRow == fromRow + 1;
}

bool BlackPawnMovementRule::isLegalCapture(int fromRow, int fromCol, int toRow, int toCol) const {
    return std::abs(toCol - fromCol) == 1 && toRow == fromRow + 1;
}
