#include "logic/rules/RuleEngine.h"
#include "logic/rules/PieceReachability.h"
#include "logic/model/Position.h"

MoveValidation validateMove(const Board& board, int fromRow, int fromCol, int toRow, int toCol) {
    if (!board.inBounds(fromRow, fromCol) || !board.inBounds(toRow, toCol)) {
        return MoveValidation{false, MoveRejectionReason::OutsideBoard};
    }

    const Piece* mover = board.pieceAt(fromRow, fromCol);
    if (!mover) return MoveValidation{false, MoveRejectionReason::EmptySource};

    if (board.isSameColor(fromRow, fromCol, toRow, toCol)) {
        return MoveValidation{false, MoveRejectionReason::FriendlyDestination};
    }

    if (!pieceCanReach(board, *mover, Position{toRow, toCol})) {
        return MoveValidation{false, MoveRejectionReason::IllegalPieceMove};
    }

    return MoveValidation{true, MoveRejectionReason::Ok};
}
