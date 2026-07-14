#include "realtime/RealTimeArbiter.h"
#include <algorithm>

// True only when a real move is in flight. A jump never counts, so it can
// never block, or be blocked by, another move.
bool RealTimeArbiter::hasActiveMotion() const {
    for (const Motion& motion : motions_) {
        if (!motion.isJump()) return true;
    }
    return false;
}

void RealTimeArbiter::startMotion(Piece& piece, Position source, Position destination) {
    piece.setState(Piece::State::Moving);
    motions_.push_back(source == destination
        ? Motion::jump(piece, source, clockMs_)
        : Motion::move(piece, source, destination, clockMs_));
}

// Advances the clock and resolves every motion whose arrival time has now
// passed. Interception is checked against the *full* still-tracked motion
// list before anything is removed, so a jump landing in this same tick
// still counts as protecting its cell (see class comment).
ArrivalEvents RealTimeArbiter::advanceTime(int ms) {
    clockMs_ += ms;

    auto isProtectedByActiveJump = [&](Position cell) {
        for (const Motion& motion : motions_) {
            if (motion.isJump() && motion.destination() == cell) return true;
        }
        return false;
    };

    ArrivalEvents events;
    for (const Motion& motion : motions_) {
        if (!motion.isDueBy(clockMs_)) continue;

        if (motion.isJump()) {
            motion.piece().setState(Piece::State::Idle);
            continue;
        }

        bool intercepted = isProtectedByActiveJump(motion.destination());
        motion.piece().setState(intercepted ? Piece::State::Captured : Piece::State::Idle);
        events.push_back(ArrivalEvent{motion.source(), motion.destination(), intercepted});
    }

    motions_.erase(
        std::remove_if(motions_.begin(), motions_.end(),
            [&](const Motion& motion) { return motion.isDueBy(clockMs_); }),
        motions_.end());
    return events;
}
