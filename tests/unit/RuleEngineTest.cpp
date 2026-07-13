#include "doctest/doctest.h"
#include "io/BoardTextFormat.h"
#include "rules/RuleEngine.h"
#include <sstream>

namespace {
    bool parse(const std::string& text, Board& board) {
        std::istringstream in(text);
        std::string error;
        return BoardTextFormat::parse(in, board, error);
    }

    // Mirrors how GameEngine uses this API: the moving piece's type and
    // color are read from the board itself, not passed in separately.
    bool legal(const Board& board, int fromRow, int fromCol, int toRow, int toCol) {
        return isLegalMove(board, fromRow, fromCol, toRow, toCol);
    }
}

TEST_CASE("rook cannot move through a blocking piece") {
    Board board;
    parse("Board:\nwR bP .\n", board);
    CHECK_FALSE(legal(board, 0, 0, 0, 2));
}

TEST_CASE("rook can move along a clear path") {
    Board board;
    parse("Board:\nwR . .\n", board);
    CHECK(legal(board, 0, 0, 0, 2));
}

TEST_CASE("bishop cannot move through a blocking piece") {
    Board board;
    parse("Board:\nwB . .\n. bP .\n. . .\n", board);
    CHECK_FALSE(legal(board, 0, 0, 2, 2));
}

TEST_CASE("bishop can move along a clear diagonal") {
    Board board;
    parse("Board:\nwB . .\n. . .\n. . .\n", board);
    CHECK(legal(board, 0, 0, 2, 2));
}

TEST_CASE("queen is blocked orthogonally by a piece in the way") {
    Board board;
    parse("Board:\nwQ bP .\n", board);
    CHECK_FALSE(legal(board, 0, 0, 0, 2));
}

TEST_CASE("queen is blocked diagonally by a piece in the way") {
    Board board;
    parse("Board:\nwQ . .\n. bP .\n. . .\n", board);
    CHECK_FALSE(legal(board, 0, 0, 2, 2));
}

TEST_CASE("knight can jump over blocking pieces surrounding it") {
    Board board;
    parse("Board:\nwN bP bP\nbP bP .\n", board);
    CHECK(legal(board, 0, 0, 1, 2));
}

TEST_CASE("a piece cannot capture another piece of its own color") {
    Board board;
    parse("Board:\nwR . wP\n", board);
    CHECK_FALSE(legal(board, 0, 0, 0, 2));
}

TEST_CASE("a piece can capture an enemy piece at the destination") {
    Board board;
    parse("Board:\nwR . bP\n", board);
    CHECK(legal(board, 0, 0, 0, 2));
}

TEST_CASE("white pawn moves one cell upward into an empty cell") {
    Board board;
    parse("Board:\n. . .\n. wP .\n. . .\n", board);
    CHECK(legal(board, 1, 1, 0, 1));
}

TEST_CASE("white pawn cannot move two cells upward from a non-start row") {
    Board board;
    parse("Board:\n. . .\n. . .\nwP . .\n. . .\n", board);
    CHECK_FALSE(legal(board, 2, 0, 0, 0));
}

TEST_CASE("black pawn moves one cell downward into an empty cell") {
    Board board;
    parse("Board:\n. . .\n. bP .\n. . .\n", board);
    CHECK(legal(board, 1, 1, 2, 1));
}

TEST_CASE("white pawn captures an enemy piece diagonally") {
    Board board;
    parse("Board:\nbR . .\n. wP .\n. . .\n", board);
    CHECK(legal(board, 1, 1, 0, 0));
}

TEST_CASE("black pawn captures an enemy piece diagonally") {
    Board board;
    parse("Board:\n. . .\n. bP .\n. . wR\n", board);
    CHECK(legal(board, 1, 1, 2, 2));
}

TEST_CASE("white pawn cannot capture an enemy piece directly ahead") {
    Board board;
    parse("Board:\n. bR .\n. wP .\n. . .\n", board);
    CHECK_FALSE(legal(board, 1, 1, 0, 1));
}

TEST_CASE("white pawn cannot move diagonally into an empty cell") {
    Board board;
    parse("Board:\n. . .\n. wP .\n. . .\n", board);
    CHECK_FALSE(legal(board, 1, 1, 0, 2));
}

TEST_CASE("pawn cannot capture a piece of its own color diagonally") {
    Board board;
    parse("Board:\nwR . .\n. wP .\n. . .\n", board);
    CHECK_FALSE(legal(board, 1, 1, 0, 0));
}

TEST_CASE("white pawn can advance two cells from its start row with a clear path") {
    Board board;
    parse("Board:\n. . .\n. . .\n. . .\nwP . .\n", board);
    CHECK(legal(board, 3, 0, 1, 0));
}

TEST_CASE("white pawn cannot advance two cells when the path is blocked") {
    Board board;
    parse("Board:\n. . .\n. . .\nwR . .\nwP . .\n", board);
    CHECK_FALSE(legal(board, 3, 0, 1, 0));
}

TEST_CASE("white pawn cannot advance two cells onto an occupied destination") {
    Board board;
    parse("Board:\n. . .\nbR . .\n. . .\nwP . .\n", board);
    CHECK_FALSE(legal(board, 3, 0, 1, 0));
}

TEST_CASE("white pawn cannot advance two cells from a row that is not its start row") {
    Board board;
    parse("Board:\n. . .\n. . .\nwP . .\n. . .\n. . .\n", board);
    CHECK_FALSE(legal(board, 2, 0, 0, 0));
}

TEST_CASE("black pawn can advance two cells from its start row") {
    Board board;
    parse("Board:\nbP . .\n. . .\n. . .\n. . .\n", board);
    CHECK(legal(board, 0, 0, 2, 0));
}
