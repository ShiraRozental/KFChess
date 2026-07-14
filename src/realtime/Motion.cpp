#include "realtime/Motion.h"
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

Motion::Motion(Piece& piece, Position source, Position destination, long long arrivalTimeMs)
    : piece_(&piece), source_(source), destination_(destination), arrivalTimeMs_(arrivalTimeMs) {
}

Motion Motion::move(Piece& piece, Position source, Position destination, long long startTimeMs) {
    long long duration = cellDistance(source, destination) * kMoveDurationPerCellMs;
    return Motion(piece, source, destination, startTimeMs + duration);
}

Motion Motion::jump(Piece& piece, Position cell, long long startTimeMs) {
    return Motion(piece, cell, cell, startTimeMs + kJumpDurationMs);
}

bool Motion::isJump() const { return source_ == destination_; }
bool Motion::isDueBy(long long clockMs) const { return arrivalTimeMs_ <= clockMs; }

Piece& Motion::piece() const { return *piece_; }
Position Motion::source() const { return source_; }
Position Motion::destination() const { return destination_; }
