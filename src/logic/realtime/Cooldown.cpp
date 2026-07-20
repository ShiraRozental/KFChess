#include "logic/realtime/Cooldown.h"

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

double Cooldown::fractionElapsed() const {
    if (durationMs_ <= 0 || elapsedMs_ >= durationMs_) return 1.0;
    return static_cast<double>(elapsedMs_) / static_cast<double>(durationMs_);
}
