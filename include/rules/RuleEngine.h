#pragma once
#include <string>
#include "model/Board.h"

// Result of checking whether a move is legal by chess-shape/occupancy rules
// alone. `reason` is a stable, machine-checkable identifier — "ok" when
// legal, or the specific rule that rejected the move — so callers (and their
// tests) don't have to re-derive why a move failed.
struct MoveValidation {
    bool legal;
    std::string reason;
};

namespace MoveRejectionReason {
    constexpr const char* Ok = "ok";
    constexpr const char* NoPieceAtSource = "no_piece_at_source";
    constexpr const char* IllegalShape = "illegal_shape";
    constexpr const char* PathBlocked = "path_blocked";
    constexpr const char* DestinationOccupiedByOwnPiece = "destination_occupied_by_own_piece";
}

// The single place that knows how to combine a piece's movement shape
// (MovementRule) with the board's occupancy state (Board) into one legality
// verdict. Callers (e.g. GameEngine) only need this one function — they
// don't need to know that some pieces require a clear path or that
// capturing your own color is forbidden. The moving piece's type and color
// are read from board itself, so callers never look them up separately.
MoveValidation validateMove(const Board& board, int fromRow, int fromCol, int toRow, int toCol);
