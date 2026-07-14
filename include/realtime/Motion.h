#pragma once
#include <utility>
#include <vector>
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
    // Every cell a motion passes through, in order, paired with the
    // simulated-clock time it is entered. Exposed only so the free
    // function that builds it (see Motion.cpp) can name its return type —
    // callers of Motion have no need to construct or inspect one directly.
    using Waypoints = std::vector<std::pair<Position, long long>>;

    static Motion move(Piece piece, Position source, Position destination, long long startTimeMs);
    static Motion jump(Piece piece, Position cell, long long startTimeMs);

    bool isJump() const;
    bool isDueBy(long long clockMs) const;
    Position currentCellAt(long long clockMs) const;

    long long startTimeMs() const;
    // Every waypoint except the source (nothing "arrives" there — it's
    // already vacated at start), each paired with its entry time. Empty
    // once a motion has been collapsed all the way back to its own source
    // (see stopBeforeReaching) — such a motion has nothing left to arrive
    // anywhere with.
    Waypoints arrivalPoints() const;
    // Same-color near-collision: the motion never reaches cell — it stops
    // at the waypoint just before it (its own source, for a shape with no
    // intermediate cells).
    void stopBeforeReaching(Position cell);
    // Opposite-color collision: the motion is captured exactly at cell —
    // it becomes the new (earlier) destination.
    void stopAtReaching(Position cell);

    Piece& piece();
    const Piece& piece() const;
    Position source() const;
    Position destination() const;

private:
    Motion(Piece piece, Position source, Position destination, Waypoints waypoints);

    Piece piece_;
    Position source_;
    Position destination_;

    // (source, start time) ... (destination, arrival time). At least one
    // entry always; only one left once stopBeforeReaching collapses a
    // motion back to its own source.
    Waypoints waypoints_;
};
