#include "doctest/doctest.h"
#include "engine/GameSnapshot.h"

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

TEST_CASE("mutating the original board after the snapshot is taken does not affect the snapshot") {
    Board board(1, 1);
    board.addPiece(0, 0, Piece(1, PieceColor::White, PieceType::King, Position{0, 0}));
    GameSnapshot snapshot(board, false, std::nullopt);

    board.removePiece(0, 0);

    CHECK(board.pieceAt(0, 0) == nullptr);
    CHECK(snapshot.board().pieceAt(0, 0) != nullptr);
}
