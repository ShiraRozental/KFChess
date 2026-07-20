#pragma once
#include <set>
#include "logic/model/Board.h"
#include "logic/model/Piece.h"
#include "logic/model/Position.h"

// Every cell a piece could move to right now, given the board's occupancy:
// correct movement shape, and a clear path where the piece's rule requires
// one. A cell occupied by an enemy piece is a legal destination (it would be
// captured on arrival); a cell occupied by a friendly piece never is. This
// function only reports which cells qualify — it never captures, removes,
// moves, or otherwise changes any piece.
std::set<Position> legalDestinationsFor(const Board& board, const Piece& piece);
