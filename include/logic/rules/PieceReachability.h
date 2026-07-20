#pragma once
#include "logic/model/Board.h"
#include "logic/model/Piece.h"
#include "logic/model/Position.h"

// Given a mover's own position and kind, and the board's current occupancy,
// decides whether a single destination cell is reachable right now: correct
// movement shape (a plain move, a capture, or a pawn's opening double step),
// and a clear path where the piece's rule requires one.
//
// Does not check board bounds and does not exclude friendly-occupied
// destinations — callers are expected to have checked bounds already, and to
// decide separately what to do about a friendly destination (RuleEngine
// reports it as its own rejection reason; LegalDestinations simply excludes
// it before ever calling this function).
bool pieceCanReach(const Board& board, const Piece& mover, Position to);
