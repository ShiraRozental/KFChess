#pragma once
#include "model/Piece.h"
#include "model/Position.h"

// A single move or jump currently in flight. A jump is represented as a
// motion whose source equals its destination — Motion knows how to tell
// the two apart and how long each kind takes to complete, so
// RealTimeArbiter only has to track a collection of these, not two
// parallel move/jump mechanisms.
class Motion {
public:
    static Motion move(Piece& piece, Position source, Position destination, long long startTimeMs);
    static Motion jump(Piece& piece, Position cell, long long startTimeMs);

    bool isJump() const;
    bool isDueBy(long long clockMs) const;

    Piece& piece() const;
    Position source() const;
    Position destination() const;

private:
    Motion(Piece& piece, Position source, Position destination, long long arrivalTimeMs);

    Piece* piece_;
    Position source_;
    Position destination_;
    long long arrivalTimeMs_;
};
