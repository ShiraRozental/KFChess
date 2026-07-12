#include "MoveLegality.h"
#include "MovementRuleFactory.h"

bool isLegalMove(const Board& board, PieceType type, int fromRow, int fromCol, int toRow, int toCol) {
    auto color = board.colorAt(fromRow, fromCol);
    if (!color.has_value()) return false;

    const MovementRule& rule = movementRuleFor(type, *color);
    bool isCapture = !board.isEmpty(toRow, toCol);
    bool shapeIsLegal = isCapture
        ? rule.isLegalCapture(fromRow, fromCol, toRow, toCol)
        : rule.isLegalMove(fromRow, fromCol, toRow, toCol);
    if (!shapeIsLegal) return false;

    if (rule.requiresClearPath() && !board.isPathClear(fromRow, fromCol, toRow, toCol)) return false;

    if (board.isSameColor(fromRow, fromCol, toRow, toCol)) return false;

    return true;
}
