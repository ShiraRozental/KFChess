#include "view/PieceView.h"

PieceView::PieceView(const std::map<std::string, SpriteAnimation>& animations,
                     Piece::State initialState)
    : animations_(&animations), animator_(initialState) {
}

void PieceView::advance(Piece::State state, long long dtMs) {
    animator_.advance(state, dtMs);
}

const Img& PieceView::currentFrame() const {
    return animations_->at(animator_.stateKey()).frameAt(animator_.elapsedMs());
}
