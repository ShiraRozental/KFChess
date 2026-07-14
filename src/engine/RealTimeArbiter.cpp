#include "engine/RealTimeArbiter.h"
#include <algorithm>
#include <cstdlib>

namespace {
    // CELL_SIZE (100px) / PIECE_SPEED (100px/s) = 1000ms per cell of
    // Chebyshev distance travelled. A jump's short defensive window uses
    // the same duration.
    constexpr long long kMoveDurationPerCellMs = 1000;
    constexpr long long kJumpDurationMs = 1000;

    // Chebyshev distance: straight and diagonal steps both count as one
    // cell of travel, matching how every piece's shape is defined in terms
    // of row/column deltas (not Euclidean pixel distance).
    int cellDistance(Position from, Position to) {
        return std::max(std::abs(to.row - from.row), std::abs(to.col - from.col));
    }
}

// True only when a real move (source != destination) is in flight. A jump
// never counts, so it can never block, or be blocked by, another move.
bool RealTimeArbiter::hasActiveMotion() const {
    for (const Motion& motion : motions_) {
        if (motion.source != motion.destination) return true;
    }
    return false;
}

void RealTimeArbiter::startMotion(Piece& piece, Position source, Position destination) {
    piece.setState(Piece::State::Moving);
    long long duration = source == destination
        ? kJumpDurationMs
        : cellDistance(source, destination) * kMoveDurationPerCellMs;
    motions_.push_back(Motion{&piece, source, destination, clockMs_ + duration});
}

// Advances the clock and resolves every motion whose arrival time has now
// passed. Interception is checked against the *full* still-tracked motion
// list before anything is removed, so a jump landing in this same tick
// still counts as protecting its cell (see class comment).
ArrivalEvents RealTimeArbiter::advanceTime(int ms) {
    clockMs_ += ms;

    auto isDue = [&](const Motion& motion) { return motion.arrivalTimeMs <= clockMs_; };
    auto isJump = [](const Motion& motion) { return motion.source == motion.destination; };
    auto isProtectedByActiveJump = [&](Position cell) {
        for (const Motion& motion : motions_) {
            if (isJump(motion) && motion.destination == cell) return true;
        }
        return false;
    };

    ArrivalEvents events;
    for (Motion& motion : motions_) {
        if (!isDue(motion)) continue;

        if (isJump(motion)) {
            motion.piece->setState(Piece::State::Idle);
            continue;
        }

        bool intercepted = isProtectedByActiveJump(motion.destination);
        motion.piece->setState(intercepted ? Piece::State::Captured : Piece::State::Idle);
        events.push_back(ArrivalEvent{motion.source, motion.destination, intercepted});
    }

    motions_.erase(std::remove_if(motions_.begin(), motions_.end(), isDue), motions_.end());
    return events;
}
