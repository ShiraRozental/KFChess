#pragma once
#include "model/Piece.h"
#include "model/Position.h"

// A single move or jump currently in flight. A jump is represented as a
// motion whose source equals its destination — Motion knows how to tell
// the two apart and how long each kind takes to complete, so
// RealTimeArbiter only has to track a collection of these, not two
// parallel move/jump mechanisms.
//
// Motion owns its Piece for the duration of the flight: an in-flight piece
// is not on the Board at all, so there is no board slot for a pointer to
// dangle into. The piece is handed back to the caller through the
// ArrivalEvent that resolves this motion.
class Motion {
public:
    static Motion move(Piece piece, Position source, Position destination, long long startTimeMs);
    static Motion jump(Piece piece, Position cell, long long startTimeMs);

    bool isJump() const;
    bool isDueBy(long long clockMs) const;

    Piece& piece();
    const Piece& piece() const;
    Position source() const;
    Position destination() const;

private:
    Motion(Piece piece, Position source, Position destination, long long arrivalTimeMs);

    Piece piece_;
    Position source_;
    Position destination_;
    long long arrivalTimeMs_;
};
