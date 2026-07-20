#pragma once
#include <string>
#include "logic/model/Piece.h"

const std::string& animationStateKey(Piece::State state);

// Tracks one piece's current animation state and how long it has been in that
// state, resetting elapsed time whenever the state changes. Pure view logic
// (no Img/OpenCV) so it lives in the animation layer and is unit-tested; the
// OpenCV frame lookup is PieceView's job.
class PieceAnimator {
public:
    explicit PieceAnimator(Piece::State initialState);

    void advance(Piece::State state, long long dtMs);

    const std::string& stateKey() const;
    long long elapsedMs() const;

private:
    Piece::State state_;
    long long elapsedMs_ = 0;
};
