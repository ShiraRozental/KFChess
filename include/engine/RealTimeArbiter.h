#pragma once
#include <vector>
#include "model/Piece.h"
#include "model/Position.h"

// A single motion that has arrived: the mover's origin/destination cells,
// and whether it was intercepted by a still-active jump at the destination
// (the mover never lands; the defender survives untouched) instead of
// landing normally.
struct ArrivalEvent {
    Position from;
    Position to;
    bool intercepted;
};

using ArrivalEvents = std::vector<ArrivalEvent>;

// Owns all timing/motion bookkeeping for pieces currently in flight (moving
// or jumping). Never touches Board — it only mutates the Piece objects it
// was explicitly handed via startMotion, and reasons about interception by
// comparing its own tracked motions' cells against each other.
//
// A jump is represented as a motion whose source equals its destination.
// hasActiveMotion() only ever reports true for a real move (source !=
// destination) — a jump never blocks, and is never blocked by, another
// piece's move (Jump is a defensive action and must stay usable while
// another piece is mid-move).
//
// Holding a raw Piece* per motion is safe under two invariants the caller
// (GameEngine) must uphold: only one move may be active at a time, and a
// move arriving at a still-jumping piece's cell is always intercepted
// rather than capturing it. Together these guarantee a piece with an
// active motion can never be removed from the board while that motion is
// still pending.
class RealTimeArbiter {
public:
    bool hasActiveMotion() const;
    void startMotion(Piece& piece, Position source, Position destination);
    ArrivalEvents advanceTime(int ms);

private:
    struct Motion {
        Piece* piece;
        Position source;
        Position destination;
        long long arrivalTimeMs;
    };

    long long clockMs_ = 0;
    std::vector<Motion> motions_;
};
