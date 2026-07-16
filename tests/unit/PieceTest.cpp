#include "doctest/doctest.h"
#include "model/Piece.h"

TEST_CASE("constructing a piece stores its id, color, kind, and cell") {
    Piece piece(7, PieceColor::White, PieceType::Knight, Position{2, 3});
    CHECK(piece.id() == 7);
    CHECK(piece.color() == PieceColor::White);
    CHECK(piece.kind() == PieceType::Knight);
    CHECK(piece.cell() == Position{2, 3});
}

TEST_CASE("a newly constructed piece starts in the idle state") {
    Piece piece(1, PieceColor::Black, PieceType::Pawn, Position{0, 0});
    CHECK(piece.state() == Piece::State::Idle);
}

TEST_CASE("moveTo updates the cell without changing the state") {
    Piece piece(1, PieceColor::White, PieceType::Rook, Position{0, 0});
    piece.setState(Piece::State::Moving);
    piece.moveTo(Position{4, 5});
    CHECK(piece.cell() == Position{4, 5});
    CHECK(piece.state() == Piece::State::Moving);
}

TEST_CASE("setState updates the state without changing the cell") {
    Piece piece(1, PieceColor::Black, PieceType::Queen, Position{6, 1});
    piece.setState(Piece::State::Captured);
    CHECK(piece.state() == Piece::State::Captured);
    CHECK(piece.cell() == Position{6, 1});
}

TEST_CASE("setState can transition through idle, moving, and captured") {
    Piece piece(1, PieceColor::White, PieceType::King, Position{0, 0});
    CHECK(piece.state() == Piece::State::Idle);
    piece.setState(Piece::State::Moving);
    CHECK(piece.state() == Piece::State::Moving);
    piece.setState(Piece::State::Captured);
    CHECK(piece.state() == Piece::State::Captured);
}

TEST_CASE("setState round-trips the jump and rest states") {
    Piece piece(1, PieceColor::White, PieceType::Bishop, Position{3, 3});
    piece.setState(Piece::State::Jumping);
    CHECK(piece.state() == Piece::State::Jumping);
    piece.setState(Piece::State::ShortRest);
    CHECK(piece.state() == Piece::State::ShortRest);
    piece.setState(Piece::State::LongRest);
    CHECK(piece.state() == Piece::State::LongRest);
}
