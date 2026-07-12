#pragma once
#include "model/Board.h"
#include "model/PieceType.h"

// The single place that knows how to combine a piece's movement shape
// (MovementRule) with the board's occupancy state (Board) into one legality
// verdict. Callers (e.g. GameEngine) only need this one function — they
// don't need to know that some pieces require a clear path or that
// capturing your own color is forbidden.
bool isLegalMove(const Board& board, PieceType type, int fromRow, int fromCol, int toRow, int toCol);
