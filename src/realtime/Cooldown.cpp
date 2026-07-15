#include "realtime/Cooldown.h"

Cooldown::Cooldown(PieceId pieceId, Position cell, long long durationMs)
    : pieceId_(pieceId), cell_(cell), durationMs_(durationMs) {
}

PieceId Cooldown::pieceId() const { return pieceId_; }
Position Cooldown::cell() const { return cell_; }

void Cooldown::advance(long long deltaMs) {
    elapsedMs_ += deltaMs;
}

bool Cooldown::hasElapsed() const {
    return elapsedMs_ >= durationMs_;
}
