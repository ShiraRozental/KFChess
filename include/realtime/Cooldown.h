#pragma once
#include "model/Piece.h"
#include "model/Position.h"

class Cooldown {
public:
    Cooldown(PieceId pieceId, Position cell, long long durationMs);

    PieceId pieceId() const;
    Position cell() const;

    void advance(long long deltaMs);
    bool hasElapsed() const;

private:
    PieceId pieceId_;
    Position cell_;
    long long durationMs_;
    long long elapsedMs_ = 0;
};
