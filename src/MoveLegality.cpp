#include "MoveLegality.h"
#include "MovementRuleFactory.h"

bool isLegalMove(const Board& board, PieceType type, int fromRow, int fromCol, int toRow, int toCol) {
    auto color = board.colorAt(fromRow, fromCol);
    if (!color.has_value()) return false;

    const MovementRule& rule = movementRuleFor(type, *color);
    bool isCapture = !board.isEmpty(toRow, toCol);
    bool isDoubleMove = !isCapture &&
        !rule.isLegalMove(fromRow, fromCol, toRow, toCol) &&
        rule.isLegalDoubleMove(fromRow, fromCol, toRow, toCol, board.rowCount());
    bool shapeIsLegal = isCapture
        ? rule.isLegalCapture(fromRow, fromCol, toRow, toCol)
        : (rule.isLegalMove(fromRow, fromCol, toRow, toCol) || isDoubleMove);
    if (!shapeIsLegal) return false;

    // A two-cell opening advance always requires a clear path (the square
    // jumped over must be empty), regardless of whether this piece type
    // otherwise cares about path-blocking.
    bool needsClearPath = rule.requiresClearPath() || isDoubleMove;
    if (needsClearPath && !board.isPathClear(fromRow, fromCol, toRow, toCol)) return false;

    if (board.isSameColor(fromRow, fromCol, toRow, toCol)) return false;

    return true;
}
