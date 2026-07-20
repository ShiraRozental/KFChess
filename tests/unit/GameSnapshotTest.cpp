#include "doctest/doctest.h"
#include "logic/engine/GameSnapshot.h"

TEST_CASE("a snapshot exposes the board it was constructed with") {
    Board board(1, 1);
    board.addPiece(0, 0, Piece(1, PieceColor::White, PieceType::King, Position{0, 0}));
    GameSnapshot snapshot(board, false, std::nullopt);
    CHECK(snapshot.board().pieceAt(0, 0) != nullptr);
}

TEST_CASE("isGameOver reflects the value passed to the constructor") {
    Board board;
    GameSnapshot inProgress(board, false, std::nullopt);
    GameSnapshot over(board, true, PieceColor::White);
    CHECK_FALSE(inProgress.isGameOver());
    CHECK(over.isGameOver());
}

TEST_CASE("winner is nullopt while the game is in progress") {
    Board board;
    GameSnapshot snapshot(board, false, std::nullopt);
    CHECK_FALSE(snapshot.winner().has_value());
}

TEST_CASE("winner reflects the color passed to the constructor") {
    Board board;
    GameSnapshot snapshot(board, true, PieceColor::Black);
    REQUIRE(snapshot.winner().has_value());
    CHECK(*snapshot.winner() == PieceColor::Black);
}

TEST_CASE("cooldownProgressOf returns the progress of a cooling piece") {
    Board board;
    GameSnapshot snapshot(board, false, std::nullopt, {{7, 0.25}});
    REQUIRE(snapshot.cooldownProgressOf(7).has_value());
    CHECK(*snapshot.cooldownProgressOf(7) == doctest::Approx(0.25));
}

TEST_CASE("cooldownProgressOf is nullopt for a piece with no cooldown") {
    Board board;
    GameSnapshot snapshot(board, false, std::nullopt, {{7, 0.25}});
    CHECK_FALSE(snapshot.cooldownProgressOf(8).has_value());
}

TEST_CASE("a snapshot built without cooldown progress reports none for any piece") {
    Board board;
    GameSnapshot snapshot(board, false, std::nullopt);
    CHECK_FALSE(snapshot.cooldownProgressOf(1).has_value());
}

TEST_CASE("mutating the original board after the snapshot is taken does not affect the snapshot") {
    Board board(1, 1);
    board.addPiece(0, 0, Piece(1, PieceColor::White, PieceType::King, Position{0, 0}));
    GameSnapshot snapshot(board, false, std::nullopt);

    board.removePiece(0, 0);

    CHECK(board.pieceAt(0, 0) == nullptr);
    CHECK(snapshot.board().pieceAt(0, 0) != nullptr);
}

TEST_CASE("inFlightPositionOf returns the fractional position of a piece in flight") {
    Board board;
    GameSnapshot snapshot(board, false, std::nullopt, {}, {{7, BoardPoint{2.0, 3.5}}});
    REQUIRE(snapshot.inFlightPositionOf(7).has_value());
    CHECK(snapshot.inFlightPositionOf(7)->row == doctest::Approx(2.0));
    CHECK(snapshot.inFlightPositionOf(7)->col == doctest::Approx(3.5));
}

TEST_CASE("inFlightPositionOf is nullopt for a piece that is not in flight") {
    Board board;
    GameSnapshot snapshot(board, false, std::nullopt);
    CHECK_FALSE(snapshot.inFlightPositionOf(7).has_value());
}
