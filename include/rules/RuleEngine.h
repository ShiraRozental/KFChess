#pragma once
#include "model/Board.h"

// The single place that knows how to combine a piece's movement shape
// (MovementRule) with the board's occupancy state (Board) into one legality
// verdict. Callers (e.g. GameEngine) only need this one function — they
// don't need to know that some pieces require a clear path or that
// capturing your own color is forbidden. The moving piece's type and color
// are read from board itself, so callers never look them up separately.
bool isLegalMove(const Board& board, int fromRow, int fromCol, int toRow, int toCol);
