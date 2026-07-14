#include "rules/RuleEngine.h"
#include "rules/MovementRuleFactory.h"

MoveValidation validateMove(const Board& board, int fromRow, int fromCol, int toRow, int toCol) {
    if (!board.inBounds(fromRow, fromCol) || !board.inBounds(toRow, toCol)) {
        return MoveValidation{false, MoveRejectionReason::OutsideBoard};
    }

    const Piece* mover = board.pieceAt(fromRow, fromCol);
    if (!mover) return MoveValidation{false, MoveRejectionReason::EmptySource};

    if (board.isSameColor(fromRow, fromCol, toRow, toCol)) {
        return MoveValidation{false, MoveRejectionReason::FriendlyDestination};
    }

    const MovementRule& rule = movementRuleFor(mover->kind(), mover->color());
    bool isCapture = !board.isEmpty(toRow, toCol);
    bool isDoubleMove = !isCapture &&
        !rule.isLegalMove(fromRow, fromCol, toRow, toCol) &&
        rule.isLegalDoubleMove(fromRow, fromCol, toRow, toCol, board.rowCount());
    bool shapeIsLegal = isCapture
        ? rule.isLegalCapture(fromRow, fromCol, toRow, toCol)
        : (rule.isLegalMove(fromRow, fromCol, toRow, toCol) || isDoubleMove);
    if (!shapeIsLegal) return MoveValidation{false, MoveRejectionReason::IllegalPieceMove};

    // A two-cell opening advance always requires a clear path (the square
    // jumped over must be empty), regardless of whether this piece type
    // otherwise cares about path-blocking.
    bool needsClearPath = rule.requiresClearPath() || isDoubleMove;
    if (needsClearPath && !board.isPathClear(fromRow, fromCol, toRow, toCol)) {
        return MoveValidation{false, MoveRejectionReason::IllegalPieceMove};
    }

    return MoveValidation{true, MoveRejectionReason::Ok};
}
