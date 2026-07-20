#pragma once
#include <utility>
#include <vector>
#include "logic/model/BoardPoint.h"
#include "logic/model/Piece.h"
#include "logic/model/Position.h"


class Motion {
public:
    
    using Waypoints = std::vector<std::pair<Position, long long>>;

    static Motion move(Piece piece, Position source, Position destination, long long startTimeMs);
    static Motion jump(Piece piece, Position cell, long long startTimeMs);

    bool isJump() const;
    bool isDueBy(long long clockMs) const;
    Position currentCellAt(long long clockMs) const;
    BoardPoint fractionalPositionAt(long long clockMs) const;

    long long startTimeMs() const;
   
    Waypoints arrivalPoints() const;
    
    void stopBeforeReaching(Position cell);
    
    void stopAtReaching(Position cell);

    Piece& piece();
    const Piece& piece() const;
    Position source() const;
    Position destination() const;

private:
    Motion(Piece piece, Position source, Position destination, Waypoints waypoints, bool isJump);

    Piece piece_;
    Position source_;
    Position destination_;
    bool isJump_;

    Waypoints waypoints_;
};
