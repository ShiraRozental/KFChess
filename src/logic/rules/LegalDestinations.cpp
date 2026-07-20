#include "logic/rules/LegalDestinations.h"
#include "logic/rules/PieceReachability.h"

std::set<Position> legalDestinationsFor(const Board& board, const Piece& piece) {
    std::set<Position> destinations;
    Position from = piece.cell();

    for (int row = 0; row < board.rowCount(); ++row) {
        for (int col = 0; col < board.colCount(); ++col) {
            if (row == from.row && col == from.col) continue;
            if (board.isSameColor(from.row, from.col, row, col)) continue;

            Position to{row, col};
            if (pieceCanReach(board, piece, to)) {
                destinations.insert(to);
            }
        }
    }

    return destinations;
}
