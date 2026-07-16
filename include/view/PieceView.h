#pragma once
#include <map>
#include <string>
#include "animation/PieceAnimator.h"
#include "view/SpriteAnimation.h"

class PieceView {
public:
    PieceView(const std::map<std::string, SpriteAnimation>& animations,
              Piece::State initialState);

    void advance(Piece::State state, long long dtMs);
    const Img& currentFrame() const;

private:
    const std::map<std::string, SpriteAnimation>* animations_;
    PieceAnimator animator_;
};
