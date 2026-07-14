#include "realtime/RealTimeArbiter.h"
#include <algorithm>
#include <utility>

// True only when a real move is in flight. A jump never counts, so it can
// never block, or be blocked by, another move.
bool RealTimeArbiter::hasActiveMotion() const {
    for (const Motion& motion : motions_) {
        if (!motion.isJump()) return true;
    }
    return false;
}

// True while a jump is still airborne over the given cell — used by
// GameEngine to report a clear motion_in_progress rejection when a
// mid-jump piece is commanded.
bool RealTimeArbiter::hasJumpAt(Position cell) const {
    return jumpColorAt(cell).has_value();
}

// The color of the piece jumping over the given cell, or nullopt if no
// jump is airborne there. Lets GameEngine keep enforcing
// friendly_destination for a defender RuleEngine can no longer see (an
// airborne piece is off the board), and lets interception below apply only
// to enemies.
std::optional<PieceColor> RealTimeArbiter::jumpColorAt(Position cell) const {
    for (const Motion& motion : motions_) {
        if (motion.isJump() && motion.destination() == cell) return motion.piece().color();
    }
    return std::nullopt;
}

void RealTimeArbiter::startMotion(Piece piece, Position source, Position destination) {
    piece.setState(Piece::State::Moving);
    motions_.push_back(source == destination
        ? Motion::jump(std::move(piece), source, clockMs_)
        : Motion::move(std::move(piece), source, destination, clockMs_));
}

// Advances the clock and resolves every motion whose arrival time has now
// passed, handing each piece back through an ArrivalEvent. Interception is
// checked against the *full* still-tracked motion list before anything is
// removed, so a jump landing in this same tick still counts as protecting
// its cell (see class comment).
ArrivalEvents RealTimeArbiter::advanceTime(int ms) {
    clockMs_ += ms;

    ArrivalEvents events;
    for (Motion& motion : motions_) {
        if (!motion.isDueBy(clockMs_)) continue;

        // Interception only protects against enemies: a jump was never a
        // way to capture a friend that arrives at the same cell.
        std::optional<PieceColor> jumperColor = jumpColorAt(motion.destination());
        bool intercepted = !motion.isJump()
            && jumperColor.has_value() && *jumperColor != motion.piece().color();
        Piece& piece = motion.piece();
        if (intercepted) {
            piece.setState(Piece::State::Captured);
        } else {
            piece.moveTo(motion.destination());
            piece.setState(Piece::State::Idle);
        }
        events.push_back(ArrivalEvent{piece, motion.source(), motion.destination(), intercepted});
    }

    motions_.erase(
        std::remove_if(motions_.begin(), motions_.end(),
            [&](const Motion& motion) { return motion.isDueBy(clockMs_); }),
        motions_.end());
    return events;
}

// Copies of every in-flight piece with its current display cell (its
// actual position along its trajectory at the current clock, not always
// its source), so GameEngine can compose a snapshot in which mid-motion
// pieces never disappear and are shown where they really are.
std::vector<InFlightPiece> RealTimeArbiter::inFlightPieces() const {
    std::vector<InFlightPiece> fliers;
    fliers.reserve(motions_.size());
    for (const Motion& motion : motions_) {
        fliers.push_back(InFlightPiece{motion.piece(), motion.currentCellAt(clockMs_)});
    }
    return fliers;
}
