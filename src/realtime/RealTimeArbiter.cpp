#include "realtime/RealTimeArbiter.h"
#include <algorithm>
#include <optional>
#include <utility>

namespace {
    struct PairConflict {
        Position cell;
        long long timeMs;
        size_t loserIndex;
        bool captureLoser;
    };

    // A jump occupies its own cell for its whole flight, not just at its two
    // waypoints — so it's checked against every arrival point of the other
    // motion, not matched waypoint-for-waypoint like two ordinary motions.
    std::optional<PairConflict> jumpConflict(const Motion& jumper, const Motion& mover, size_t moverIndex) {
        long long jumpEnd = jumper.arrivalPoints().front().second;
        for (const auto& [cell, time] : mover.arrivalPoints()) {
            if (cell != jumper.destination()) continue;
            if (time < jumper.startTimeMs() || time > jumpEnd) continue;
            bool enemy = mover.piece().color() != jumper.piece().color();
            return PairConflict{cell, time, moverIndex, enemy};
        }
        return std::nullopt;
    }

    // Same cell, both colors: the LATER arriver stops short of it (never
    // reaches it). Opposite colors: the LATER arriver captures the EARLIER
    // one (the earlier one is the loser, the later one wins and continues).
    std::optional<PairConflict> ordinaryConflict(
        const Motion& a, const Motion& b, size_t indexA, size_t indexB) {
        std::optional<PairConflict> best;
        for (const auto& [cellA, timeA] : a.arrivalPoints()) {
            for (const auto& [cellB, timeB] : b.arrivalPoints()) {
                if (cellA != cellB) continue;
                long long resolveTime = std::max(timeA, timeB);
                if (best && resolveTime >= best->timeMs) continue;

                bool aIsEarlier;
                if (timeA != timeB) {
                    aIsEarlier = timeA < timeB;
                } else if (a.startTimeMs() != b.startTimeMs()) {
                    aIsEarlier = a.startTimeMs() < b.startTimeMs();
                } else {
                    aIsEarlier = indexA < indexB;
                }
                size_t earlier = aIsEarlier ? indexA : indexB;
                size_t later = aIsEarlier ? indexB : indexA;

                bool enemy = a.piece().color() != b.piece().color();
                best = PairConflict{cellA, resolveTime, enemy ? earlier : later, enemy};
            }
        }
        return best;
    }

    std::optional<PairConflict> conflictBetween(
        const Motion& a, const Motion& b, size_t indexA, size_t indexB) {
        // A motion already collapsed down to just its source (e.g. a knight
        // truncated to nothing) has nothing left to arrive anywhere with.
        if (a.arrivalPoints().empty() || b.arrivalPoints().empty()) return std::nullopt;
        if (a.isJump() && b.isJump()) return std::nullopt;
        if (a.isJump()) return jumpConflict(a, b, indexB);
        if (b.isJump()) return jumpConflict(b, a, indexA);
        return ordinaryConflict(a, b, indexA, indexB);
    }
}

// True only when a real move is in flight. A jump never counts, so it can
// never block, or be blocked by, another move.
bool RealTimeArbiter::hasActiveMotion() const {
    for (const Motion& motion : motions_) {
        if (!motion.isJump()) return true;
    }
    return false;
}

bool RealTimeArbiter::hasMotionFrom(Position cell) const {
    for (const Motion& motion : motions_) {
        if (motion.source() == cell) return true;
    }
    return false;
}

void RealTimeArbiter::startMotion(Piece piece, Position source, Position destination) {
    piece.setState(Piece::State::Moving);
    motions_.push_back(source == destination
        ? Motion::jump(std::move(piece), source, clockMs_)
        : Motion::move(std::move(piece), source, destination, clockMs_));
}

// Repeatedly finds and applies the single earliest still-unresolved
// conflict among all pairs of tracked motions, until none remain at or
// before the current clock. Applying a conflict collapses the loser's
// trajectory at or just before the collision cell, which is what lets the
// same isDueBy/erase machinery in advanceTime treat a piece captured or
// stopped mid-flight exactly like one that arrived normally.
void RealTimeArbiter::resolveConflicts(std::vector<bool>& capturedMidFlight) {
    while (true) {
        std::optional<PairConflict> earliest;
        for (size_t i = 0; i < motions_.size(); ++i) {
            if (capturedMidFlight[i]) continue;
            for (size_t j = i + 1; j < motions_.size(); ++j) {
                if (capturedMidFlight[j]) continue;
                std::optional<PairConflict> conflict = conflictBetween(motions_[i], motions_[j], i, j);
                if (!conflict || conflict->timeMs > clockMs_) continue;
                if (!earliest || conflict->timeMs < earliest->timeMs) earliest = conflict;
            }
        }
        if (!earliest) return;

        if (earliest->captureLoser) {
            capturedMidFlight[earliest->loserIndex] = true;
            motions_[earliest->loserIndex].stopAtReaching(earliest->cell);
        } else {
            motions_[earliest->loserIndex].stopBeforeReaching(earliest->cell);
        }
    }
}

// Advances the clock, resolves motion-vs-motion conflicts, then hands back
// every motion whose (possibly now-collapsed) arrival time has passed.
ArrivalEvents RealTimeArbiter::advanceTime(int ms) {
    clockMs_ += ms;

    std::vector<bool> capturedMidFlight(motions_.size(), false);
    resolveConflicts(capturedMidFlight);

    ArrivalEvents events;
    for (size_t i = 0; i < motions_.size(); ++i) {
        Motion& motion = motions_[i];
        if (!motion.isDueBy(clockMs_)) continue;

        Piece& piece = motion.piece();
        bool captured = capturedMidFlight[i];
        if (captured) {
            piece.setState(Piece::State::Captured);
        } else {
            piece.moveTo(motion.destination());
            piece.setState(Piece::State::Idle);
        }
        bool kingCaptured = captured && piece.kind() == PieceType::King;
        events.push_back(ArrivalEvent{piece, motion.source(), motion.destination(), captured, kingCaptured});
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
