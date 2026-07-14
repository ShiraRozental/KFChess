#include "doctest/doctest.h"
#include "realtime/Motion.h"

namespace {
    Piece makePiece(Position cell) {
        return Piece(1, PieceColor::White, PieceType::Rook, cell);
    }
}

TEST_CASE("a jump has the same source and destination") {
    Piece piece = makePiece(Position{1, 1});
    Motion motion = Motion::jump(piece, Position{1, 1}, 0);
    CHECK(motion.isJump());
    CHECK(motion.source() == motion.destination());
}

TEST_CASE("a move has a different source and destination and is not a jump") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 2}, 0);
    CHECK_FALSE(motion.isJump());
    CHECK(motion.source() == Position{0, 0});
    CHECK(motion.destination() == Position{0, 2});
}

TEST_CASE("a one-cell move is due at exactly 1000ms after its start time") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 1}, 500);
    CHECK_FALSE(motion.isDueBy(1499));
    CHECK(motion.isDueBy(1500));
}

TEST_CASE("a two-cell move takes twice as long as a one-cell move") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 2}, 0);
    CHECK_FALSE(motion.isDueBy(1999));
    CHECK(motion.isDueBy(2000));
}

TEST_CASE("a diagonal move uses Chebyshev distance, not Euclidean distance") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{3, 3}, 0);
    CHECK_FALSE(motion.isDueBy(2999));
    CHECK(motion.isDueBy(3000));
}

TEST_CASE("a jump is due after 1000ms regardless of distance (source equals destination)") {
    Piece piece = makePiece(Position{2, 2});
    Motion motion = Motion::jump(piece, Position{2, 2}, 0);
    CHECK_FALSE(motion.isDueBy(999));
    CHECK(motion.isDueBy(1000));
}

TEST_CASE("a motion owns its own copy of the piece, detached from the original") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 1}, 0);
    CHECK(motion.piece().id() == piece.id());

    // Mutating the motion's piece must never touch the caller's original.
    motion.piece().setState(Piece::State::Moving);
    CHECK(piece.state() == Piece::State::Idle);
}
