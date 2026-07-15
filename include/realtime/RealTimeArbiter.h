#pragma once
#include <vector>
#include "realtime/Motion.h"
#include "realtime/Cooldown.h"
#include "model/Piece.h"
#include "model/Position.h"

// A single motion that has resolved, carrying its piece back to the caller.
// A normal landing (intercepted == false) means the piece should be placed
// at `to` (for a jump, to == from). intercepted == true means the piece
// was captured while still in flight and never lands; kingCaptured is then
// true iff that piece was a king.
struct ArrivalEvent {
    Piece piece;
    Position from;
    Position to;
    bool intercepted;
    bool kingCaptured;
};

using ArrivalEvents = std::vector<ArrivalEvent>;

// A read-only view of one piece currently in flight and the cell it should
// be displayed at (its source, until motions carry a full trajectory).
struct InFlightPiece {
    Piece piece;
    Position cell;
};

// Owns every clock-driven concern of the real-time layer: in-flight
// Motions, post-landing Cooldowns, and arrival/capture resolution.
//
// hasActiveMotion() ignores jumps (Motion::isJump) — a jump never blocks,
// or is blocked by, another piece's move.
class RealTimeArbiter {
public:
    bool hasActiveMotion() const;
    bool hasMotionFrom(Position cell) const;
    void startMotion(Piece piece, Position source, Position destination);
    ArrivalEvents advanceTime(int ms);
    std::vector<InFlightPiece> inFlightPieces() const;

    void startCooldown(PieceId pieceId, Position cell, long long durationMs);
    bool isCoolingDown(PieceId pieceId) const;

private:
    void resolveConflicts(std::vector<bool>& capturedMidFlight);

    long long clockMs_ = 0;
    std::vector<Motion> motions_;
    std::vector<Cooldown> cooldowns_;
};
