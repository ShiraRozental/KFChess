#include "logic/rules/PieceReachability.h"
#include "logic/config/PieceCatalog.h"

bool pieceCanReach(const Board& board, const Piece& mover, Position to) {
    Position from = mover.cell();
    const MovementRule& rule =
        PieceCatalog::standard().definitionFor(mover.kind()).movementFor(mover.color());

    bool isCapture = !board.isEmpty(to.row, to.col);
    bool isDoubleMove = !isCapture &&
        !rule.isLegalMove(from.row, from.col, to.row, to.col) &&
        rule.isLegalDoubleMove(from.row, from.col, to.row, to.col, board.rowCount());
    bool shapeIsLegal = isCapture
        ? rule.isLegalCapture(from.row, from.col, to.row, to.col)
        : (rule.isLegalMove(from.row, from.col, to.row, to.col) || isDoubleMove);
    if (!shapeIsLegal) return false;

    // A two-cell opening advance always requires a clear path (the square
    // jumped over must be empty), regardless of whether this piece type
    // otherwise cares about path-blocking.
    bool needsClearPath = rule.requiresClearPath() || isDoubleMove;
    if (needsClearPath && !board.isPathClear(from.row, from.col, to.row, to.col)) {
        return false;
    }

    return true;
}
