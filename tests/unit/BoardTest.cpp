#include "doctest/doctest.h"
#include "logic/model/Board.h"

namespace {
    Piece makePiece(PieceId id, PieceColor color, PieceType kind, int row, int col) {
        return Piece(id, color, kind, Position{row, col});
    }
}

TEST_CASE("a freshly constructed board reports the dimensions it was given") {
    Board board(3, 2);
    CHECK(board.rowCount() == 3);
    CHECK(board.colCount() == 2);
}

TEST_CASE("inBounds respects board size") {
    Board board(2, 2);
    CHECK(board.inBounds(0, 0));
    CHECK(board.inBounds(1, 1));
    CHECK_FALSE(board.inBounds(-1, 0));
    CHECK_FALSE(board.inBounds(0, -1));
    CHECK_FALSE(board.inBounds(2, 0));
    CHECK_FALSE(board.inBounds(0, 2));
}

TEST_CASE("an empty cell returns no piece") {
    Board board(2, 2);
    CHECK(board.pieceAt(0, 0) == nullptr);
    CHECK(board.isEmpty(0, 0));
}

TEST_CASE("pieceCopyAt returns nullopt for an empty or out-of-bounds cell") {
    Board board(2, 2);
    CHECK_FALSE(board.pieceCopyAt(0, 0).has_value());
    CHECK_FALSE(board.pieceCopyAt(-1, 0).has_value());
    CHECK_FALSE(board.pieceCopyAt(5, 5).has_value());
}

TEST_CASE("pieceCopyAt returns a detached copy of an occupied cell's piece") {
    Board board(2, 2);
    board.addPiece(0, 0, makePiece(7, PieceColor::White, PieceType::Knight, 0, 0));

    std::optional<Piece> copy = board.pieceCopyAt(0, 0);
    REQUIRE(copy.has_value());
    CHECK(copy->id() == 7);
    CHECK(copy->color() == PieceColor::White);
    CHECK(copy->kind() == PieceType::Knight);

    board.removePiece(0, 0);
    CHECK(copy.has_value()); // unaffected by the board changing after the copy was taken
}

TEST_CASE("an occupied cell returns the correct piece") {
    Board board(2, 2);
    board.addPiece(0, 0, makePiece(7, PieceColor::White, PieceType::Knight, 0, 0));
    const Piece* piece = board.pieceAt(0, 0);
    REQUIRE(piece != nullptr);
    CHECK(piece->id() == 7);
    CHECK(piece->color() == PieceColor::White);
    CHECK(piece->kind() == PieceType::Knight);
    CHECK_FALSE(board.isEmpty(0, 0));
}

TEST_CASE("pieceAt returns nullptr for an out-of-bounds cell") {
    Board board(2, 2);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::King, 0, 0));
    CHECK(board.pieceAt(5, 5) == nullptr);
}

TEST_CASE("adding a piece to an out-of-bounds cell fails") {
    Board board(2, 2);
    CHECK_FALSE(board.addPiece(5, 5, makePiece(1, PieceColor::White, PieceType::King, 5, 5)));
}

TEST_CASE("adding two pieces to the same cell fails") {
    Board board(2, 2);
    CHECK(board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::King, 0, 0)));
    CHECK_FALSE(board.addPiece(0, 0, makePiece(2, PieceColor::Black, PieceType::Queen, 0, 0)));

    const Piece* piece = board.pieceAt(0, 0);
    REQUIRE(piece != nullptr);
    CHECK(piece->id() == 1);
}

TEST_CASE("moving a piece updates both its source and destination cells") {
    Board board(2, 2);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::King, 0, 0));
    board.movePiece(0, 0, 1, 1);

    CHECK(board.isEmpty(0, 0));
    const Piece* moved = board.pieceAt(1, 1);
    REQUIRE(moved != nullptr);
    CHECK(moved->id() == 1);
    CHECK(moved->cell() == Position{1, 1});
}

TEST_CASE("movePiece ignores out-of-bounds requests") {
    Board board(2, 2);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::King, 0, 0));
    board.movePiece(0, 0, 5, 5);
    CHECK_FALSE(board.isEmpty(0, 0));
}

TEST_CASE("moving onto an occupied destination captures it") {
    Board board(1, 2);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Rook, 0, 0));
    board.addPiece(0, 1, makePiece(2, PieceColor::Black, PieceType::Pawn, 0, 1));
    board.movePiece(0, 0, 0, 1);

    CHECK(board.isEmpty(0, 0));
    const Piece* survivor = board.pieceAt(0, 1);
    REQUIRE(survivor != nullptr);
    CHECK(survivor->id() == 1);
}

TEST_CASE("removing a captured piece clears its cell") {
    Board board(1, 1);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Pawn, 0, 0));
    board.removePiece(0, 0);
    CHECK(board.isEmpty(0, 0));
    CHECK(board.pieceAt(0, 0) == nullptr);
}

TEST_CASE("removePiece is a no-op for an out-of-bounds cell") {
    Board board(1, 1);
    board.removePiece(5, 5);
    CHECK(board.rowCount() == 1);
}

TEST_CASE("isSameColor compares piece colors correctly") {
    Board board(2, 2);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::King, 0, 0));
    board.addPiece(0, 1, makePiece(2, PieceColor::White, PieceType::Queen, 0, 1));
    board.addPiece(1, 0, makePiece(3, PieceColor::Black, PieceType::Rook, 1, 0));

    CHECK(board.isSameColor(0, 0, 0, 1));
    CHECK_FALSE(board.isSameColor(0, 0, 1, 0));
    CHECK_FALSE(board.isSameColor(0, 0, 1, 1)); // (1,1) is empty
}

TEST_CASE("isPathClear is true for a horizontal path with nothing in between") {
    Board board(1, 3);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Rook, 0, 0));
    CHECK(board.isPathClear(0, 0, 0, 2));
}

TEST_CASE("isPathClear is false when a piece blocks a horizontal path") {
    Board board(1, 3);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Rook, 0, 0));
    board.addPiece(0, 1, makePiece(2, PieceColor::Black, PieceType::Pawn, 0, 1));
    CHECK_FALSE(board.isPathClear(0, 0, 0, 2));
}

TEST_CASE("isPathClear is true for a diagonal path with nothing in between") {
    Board board(3, 3);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Bishop, 0, 0));
    CHECK(board.isPathClear(0, 0, 2, 2));
}

TEST_CASE("isPathClear is false when a piece blocks a diagonal path") {
    Board board(3, 3);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Bishop, 0, 0));
    board.addPiece(1, 1, makePiece(2, PieceColor::Black, PieceType::Pawn, 1, 1));
    CHECK_FALSE(board.isPathClear(0, 0, 2, 2));
}

TEST_CASE("isPathClear is vacuously true for a non-straight non-diagonal delta") {
    Board board(2, 3);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Knight, 0, 0));
    CHECK(board.isPathClear(0, 0, 1, 2));
}

TEST_CASE("promote changes the piece kind while preserving id and color") {
    Board board(2, 1);
    board.addPiece(0, 0, makePiece(9, PieceColor::White, PieceType::Pawn, 0, 0));
    board.promote(0, 0, PieceType::Queen);

    const Piece* piece = board.pieceAt(0, 0);
    REQUIRE(piece != nullptr);
    CHECK(piece->id() == 9);
    CHECK(piece->kind() == PieceType::Queen);
    CHECK(piece->color() == PieceColor::White);
}

TEST_CASE("promote does nothing on an empty cell") {
    Board board(2, 1);
    board.promote(0, 0, PieceType::Queen);
    CHECK(board.isEmpty(0, 0));
}

TEST_CASE("promote does nothing for an out-of-bounds cell") {
    Board board(2, 1);
    board.addPiece(0, 0, makePiece(1, PieceColor::White, PieceType::Pawn, 0, 0));
    board.promote(5, 5, PieceType::Queen);

    const Piece* piece = board.pieceAt(0, 0);
    REQUIRE(piece != nullptr);
    CHECK(piece->kind() == PieceType::Pawn);
}
