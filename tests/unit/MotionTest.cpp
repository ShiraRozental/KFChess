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

TEST_CASE("currentCellAt walks through every intermediate cell of a straight three-cell move") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 3}, 0);

    CHECK(motion.currentCellAt(0) == Position{0, 0});
    CHECK(motion.currentCellAt(999) == Position{0, 0});
    CHECK(motion.currentCellAt(1000) == Position{0, 1});
    CHECK(motion.currentCellAt(1999) == Position{0, 1});
    CHECK(motion.currentCellAt(2000) == Position{0, 2});
    CHECK(motion.currentCellAt(2999) == Position{0, 2});
    CHECK(motion.currentCellAt(3000) == Position{0, 3});
}

TEST_CASE("currentCellAt walks through every intermediate cell of a diagonal move") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{2, 2}, 0);

    CHECK(motion.currentCellAt(999) == Position{0, 0});
    CHECK(motion.currentCellAt(1000) == Position{1, 1});
    CHECK(motion.currentCellAt(1999) == Position{1, 1});
    CHECK(motion.currentCellAt(2000) == Position{2, 2});
}

TEST_CASE("currentCellAt reports a knight's shape only at its two endpoints, never mid-flight") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{2, 1}, 0); // Chebyshev distance 2

    CHECK(motion.currentCellAt(0) == Position{0, 0});
    CHECK(motion.currentCellAt(1999) == Position{0, 0}); // still at source: no intermediate cell exists
    CHECK(motion.currentCellAt(2000) == Position{2, 1});
}

TEST_CASE("currentCellAt reports a jump's own cell at every point in its short flight") {
    Piece piece = makePiece(Position{1, 1});
    Motion motion = Motion::jump(piece, Position{1, 1}, 0);

    CHECK(motion.currentCellAt(0) == Position{1, 1});
    CHECK(motion.currentCellAt(999) == Position{1, 1});
    CHECK(motion.currentCellAt(1000) == Position{1, 1});
}

TEST_CASE("a motion owns its own copy of the piece, detached from the original") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 1}, 0);
    CHECK(motion.piece().id() == piece.id());

    // Mutating the motion's piece must never touch the caller's original.
    motion.piece().setState(Piece::State::Moving);
    CHECK(piece.state() == Piece::State::Idle);
}

TEST_CASE("fractionalPositionAt interpolates halfway between two cells of a straight move") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 2}, 0);

    BoardPoint midFirstCell = motion.fractionalPositionAt(500);
    CHECK(midFirstCell.row == doctest::Approx(0.0));
    CHECK(midFirstCell.col == doctest::Approx(0.5));

    BoardPoint midSecondCell = motion.fractionalPositionAt(1500);
    CHECK(midSecondCell.row == doctest::Approx(0.0));
    CHECK(midSecondCell.col == doctest::Approx(1.5));
}

TEST_CASE("fractionalPositionAt slides a knight in a straight line from source to destination") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{2, 1}, 0);

    BoardPoint midway = motion.fractionalPositionAt(1000);
    CHECK(midway.row == doctest::Approx(1.0));
    CHECK(midway.col == doctest::Approx(0.5));
}

TEST_CASE("fractionalPositionAt keeps a jumping piece fixed on its own cell") {
    Piece piece = makePiece(Position{1, 1});
    Motion motion = Motion::jump(piece, Position{1, 1}, 0);

    BoardPoint midJump = motion.fractionalPositionAt(500);
    CHECK(midJump.row == doctest::Approx(1.0));
    CHECK(midJump.col == doctest::Approx(1.0));
}

TEST_CASE("fractionalPositionAt clamps to the endpoints outside the motion's time span") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 2}, 1000);

    BoardPoint beforeStart = motion.fractionalPositionAt(0);
    CHECK(beforeStart.row == doctest::Approx(0.0));
    CHECK(beforeStart.col == doctest::Approx(0.0));

    BoardPoint afterEnd = motion.fractionalPositionAt(9999);
    CHECK(afterEnd.row == doctest::Approx(0.0));
    CHECK(afterEnd.col == doctest::Approx(2.0));
}

TEST_CASE("fractionalPositionAt follows a truncated motion to its shortened destination") {
    Piece piece = makePiece(Position{0, 0});
    Motion motion = Motion::move(piece, Position{0, 0}, Position{0, 3}, 0);
    motion.stopBeforeReaching(Position{0, 2});

    BoardPoint afterEnd = motion.fractionalPositionAt(5000);
    CHECK(afterEnd.row == doctest::Approx(0.0));
    CHECK(afterEnd.col == doctest::Approx(1.0));
}
