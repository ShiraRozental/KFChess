#include "realtime/Motion.h"
#include <algorithm>
#include <cstdlib>
#include <utility>

namespace {
    
    constexpr long long kMoveDurationPerCellMs = 1000;
    constexpr long long kJumpDurationMs = 1000;

    int cellDistance(Position from, Position to) {
        return std::max(std::abs(to.row - from.row), std::abs(to.col - from.col));
    }

    bool isStraightOrDiagonal(int dRow, int dCol) {
        return dRow == 0 || dCol == 0 || std::abs(dRow) == std::abs(dCol);
    }

    Motion::Waypoints buildWaypoints(Position source, Position destination, long long startTimeMs) {
        int dRow = destination.row - source.row;
        int dCol = destination.col - source.col;

        if (!isStraightOrDiagonal(dRow, dCol)) {
            long long duration = cellDistance(source, destination) * kMoveDurationPerCellMs;
            return {{source, startTimeMs}, {destination, startTimeMs + duration}};
        }

        int stepRow = (dRow > 0) - (dRow < 0);
        int stepCol = (dCol > 0) - (dCol < 0);

        Motion::Waypoints waypoints{{source, startTimeMs}};
        Position cell = source;
        long long timeMs = startTimeMs;
        while (cell != destination) {
            cell.row += stepRow;
            cell.col += stepCol;
            timeMs += kMoveDurationPerCellMs;
            waypoints.push_back({cell, timeMs});
        }
        return waypoints;
    }
}

Motion::Motion(Piece piece, Position source, Position destination, Waypoints waypoints)
    : piece_(std::move(piece)), source_(source), destination_(destination), waypoints_(std::move(waypoints)) {
}

Motion Motion::move(Piece piece, Position source, Position destination, long long startTimeMs) {
    return Motion(std::move(piece), source, destination, buildWaypoints(source, destination, startTimeMs));
}

Motion Motion::jump(Piece piece, Position cell, long long startTimeMs) {
    return Motion(std::move(piece), cell, cell, {{cell, startTimeMs}, {cell, startTimeMs + kJumpDurationMs}});
}

bool Motion::isJump() const { return source_ == destination_; }
bool Motion::isDueBy(long long clockMs) const { return waypoints_.back().second <= clockMs; }

Position Motion::currentCellAt(long long clockMs) const {
    Position cell = waypoints_.front().first;
    for (const auto& waypoint : waypoints_) {
        if (waypoint.second > clockMs) break;
        cell = waypoint.first;
    }
    return cell;
}

BoardPoint Motion::fractionalPositionAt(long long clockMs) const {
    if (clockMs <= waypoints_.front().second) return toBoardPoint(waypoints_.front().first);
    if (clockMs >= waypoints_.back().second) return toBoardPoint(waypoints_.back().first);

    size_t next = 1;
    while (waypoints_[next].second <= clockMs) ++next;
    const auto& [fromCell, fromMs] = waypoints_[next - 1];
    const auto& [toCell, toMs] = waypoints_[next];

    double fraction = static_cast<double>(clockMs - fromMs) / static_cast<double>(toMs - fromMs);
    return BoardPoint{fromCell.row + (toCell.row - fromCell.row) * fraction,
                      fromCell.col + (toCell.col - fromCell.col) * fraction};
}

long long Motion::startTimeMs() const { return waypoints_.front().second; }

Motion::Waypoints Motion::arrivalPoints() const {
    return Waypoints(waypoints_.begin() + 1, waypoints_.end());
}

void Motion::stopBeforeReaching(Position cell) {
    auto it = std::find_if(waypoints_.begin(), waypoints_.end(),
        [&](const auto& waypoint) { return waypoint.first == cell; });
    waypoints_.erase(it, waypoints_.end());
    destination_ = waypoints_.back().first;
}

void Motion::stopAtReaching(Position cell) {
    auto it = std::find_if(waypoints_.begin(), waypoints_.end(),
        [&](const auto& waypoint) { return waypoint.first == cell; });
    waypoints_.erase(it + 1, waypoints_.end());
    destination_ = waypoints_.back().first;
}

Piece& Motion::piece() { return piece_; }
const Piece& Motion::piece() const { return piece_; }
Position Motion::source() const { return source_; }
Position Motion::destination() const { return destination_; }
