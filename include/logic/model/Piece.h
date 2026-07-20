#pragma once
#include "logic/model/PieceColor.h"
#include "logic/model/PieceType.h"
#include "logic/model/Position.h"

using PieceId = int;

// A chess piece's identity and lifecycle snapshot: who it is (id, color,
// kind), where it currently sits (cell), and what phase of its lifecycle
// it's in (state). Owns no path, destination, elapsed time, speed, or
// interpolation data — that belongs to Motion and RealTimeArbiter.
// id and color are fixed at construction and never change afterward.
class Piece {
public:
    enum class State { Idle, Moving, Jumping, ShortRest, LongRest, Captured };

    Piece(PieceId id, PieceColor color, PieceType kind, Position cell);

    PieceId id() const;
    PieceColor color() const;
    PieceType kind() const;
    Position cell() const;
    State state() const;

    void moveTo(Position cell);
    void setState(State state);

private:
    PieceId id_;
    PieceColor color_;
    PieceType kind_;
    Position cell_;
    State state_ = State::Idle;
};
