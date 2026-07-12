#include "MoveLegality.h"
#include "MovementRuleFactory.h"

bool isLegalMove(const Board& board, PieceType type, int fromRow, int fromCol, int toRow, int toCol) {
    const MovementRule& rule = movementRuleFor(type);
    if (!rule.isLegalMove(fromRow, fromCol, toRow, toCol)) return false;

    if (rule.requiresClearPath() && !board.isPathClear(fromRow, fromCol, toRow, toCol)) return false;

    if (board.isSameColor(fromRow, fromCol, toRow, toCol)) return false;

    return true;
}
