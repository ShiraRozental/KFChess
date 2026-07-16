#include "doctest/doctest.h"
#include "animation/PieceAnimator.h"

TEST_CASE("animationStateKey maps each visible piece state to its sprite folder name") {
    CHECK(animationStateKey(Piece::State::Idle) == "idle");
    CHECK(animationStateKey(Piece::State::Moving) == "move");
    CHECK(animationStateKey(Piece::State::Jumping) == "jump");
    CHECK(animationStateKey(Piece::State::ShortRest) == "short_rest");
    CHECK(animationStateKey(Piece::State::LongRest) == "long_rest");
}

TEST_CASE("animationStateKey rejects Captured, which is never rendered") {
    CHECK_THROWS_AS(animationStateKey(Piece::State::Captured), std::invalid_argument);
}

TEST_CASE("a new animator starts in its initial state with no elapsed time") {
    PieceAnimator animator(Piece::State::Moving);
    CHECK(animator.stateKey() == "move");
    CHECK(animator.elapsedMs() == 0);
}

TEST_CASE("advancing with the same state accumulates elapsed time") {
    PieceAnimator animator(Piece::State::Moving);
    animator.advance(Piece::State::Moving, 300);
    animator.advance(Piece::State::Moving, 200);
    CHECK(animator.elapsedMs() == 500);
    CHECK(animator.stateKey() == "move");
}

TEST_CASE("advancing with a different state switches animation and resets elapsed time") {
    PieceAnimator animator(Piece::State::Moving);
    animator.advance(Piece::State::Moving, 400);
    animator.advance(Piece::State::LongRest, 100);
    CHECK(animator.stateKey() == "long_rest");
    CHECK(animator.elapsedMs() == 0);
}

TEST_CASE("elapsed time accumulates again from zero after a state change") {
    PieceAnimator animator(Piece::State::Moving);
    animator.advance(Piece::State::LongRest, 100);
    animator.advance(Piece::State::LongRest, 50);
    CHECK(animator.elapsedMs() == 50);
}
