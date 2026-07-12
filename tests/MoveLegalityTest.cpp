#include "doctest/doctest.h"
#include "MoveLegality.h"
#include <sstream>

namespace {
    bool parse(const std::string& text, Board& board) {
        std::istringstream in(text);
        std::string error;
        return Board::fromStream(in, board, error);
    }

    // Looks up the piece type at (fromRow, fromCol) and checks legality of
    // moving it to (toRow, toCol), mirroring how Game uses this API.
    bool legal(const Board& board, int fromRow, int fromCol, int toRow, int toCol) {
        auto type = board.pieceTypeAt(fromRow, fromCol);
        REQUIRE(type.has_value());
        return isLegalMove(board, *type, fromRow, fromCol, toRow, toCol);
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
