#include "animation/PieceAnimator.h"
#include "animation/AnimationStateNames.h"
#include <stdexcept>

const std::string& animationStateKey(Piece::State state) {
    switch (state) {
        case Piece::State::Idle:      return AnimationStateName::Idle;
        case Piece::State::Moving:    return AnimationStateName::Move;
        case Piece::State::Jumping:   return AnimationStateName::Jump;
        case Piece::State::ShortRest: return AnimationStateName::ShortRest;
        case Piece::State::LongRest:  return AnimationStateName::LongRest;
        case Piece::State::Captured:  break;
    }
    throw std::invalid_argument("Captured pieces are not rendered and have no animation state");
}

PieceAnimator::PieceAnimator(Piece::State initialState) : state_(initialState) {
}

void PieceAnimator::advance(Piece::State state, long long dtMs) {
    if (state == state_) {
        elapsedMs_ += dtMs;
        return;
    }
    state_ = state;
    elapsedMs_ = 0;
}

const std::string& PieceAnimator::stateKey() const {
    return animationStateKey(state_);
}

long long PieceAnimator::elapsedMs() const {
    return elapsedMs_;
}
