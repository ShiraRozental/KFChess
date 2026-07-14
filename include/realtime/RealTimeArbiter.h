#pragma once
#include <optional>
#include <vector>
#include "realtime/Motion.h"
#include "model/Piece.h"
#include "model/PieceColor.h"
#include "model/Position.h"

// A single motion that has resolved, carrying its piece back to the caller.
// A normal landing (intercepted == false) means the piece should be placed
// at `to` (for a jump, to == from). intercepted == true means the piece
// was captured while still in flight — by an active jump's defense, or by
// losing a collision to another in-flight piece — and never lands.
struct ArrivalEvent {
    Piece piece;
    Position from;
    Position to;
    bool intercepted;
};

using ArrivalEvents = std::vector<ArrivalEvent>;

// A read-only view of one piece currently in flight and the cell it should
// be displayed at (its source, until motions carry a full trajectory).
struct InFlightPiece {
    Piece piece;
    Position cell;
};

// Owns all timing/motion bookkeeping for pieces currently in flight (moving
// or jumping). Never touches Board — while a piece is in flight its Motion
// owns it outright (the piece is off the board entirely), and it is handed
// back through the ArrivalEvent that resolves the motion.
//
// hasActiveMotion() only ever reports true for a real move (see
// Motion::isJump) — a jump never blocks, and is never blocked by, another
// piece's move (Jump is a defensive action and must stay usable while
// another piece is mid-move).
class RealTimeArbiter {
public:
    bool hasActiveMotion() const;
    bool hasJumpAt(Position cell) const;
    std::optional<PieceColor> jumpColorAt(Position cell) const;
    void startMotion(Piece piece, Position source, Position destination);
    ArrivalEvents advanceTime(int ms);
    std::vector<InFlightPiece> inFlightPieces() const;

private:
    void resolveConflicts(std::vector<bool>& capturedMidFlight);

    long long clockMs_ = 0;
    std::vector<Motion> motions_;
};
