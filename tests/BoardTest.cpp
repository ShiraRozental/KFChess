#include "doctest/doctest.h"
#include "Board.h"
#include <sstream>

namespace {
    bool parse(const std::string& text, Board& board, std::string& error) {
        std::istringstream in(text);
        return Board::fromStream(in, board, error);
    }

    std::string printed(const Board& board) {
        std::ostringstream out;
        board.print(out);
        return out.str();
    }
}

TEST_CASE("valid board parses and infers dimensions") {
    Board board;
    std::string error;
    CHECK(parse("Board:\nwK . .\n. . .\n. . .\n", board, error));
    CHECK(printed(board) == "wK . .\n. . .\n. . .");
}

TEST_CASE("unknown token is rejected") {
    Board board;
    std::string error;
    CHECK_FALSE(parse("Board:\nwK xZ\n. .\n", board, error));
    CHECK(error == "ERROR UNKNOWN_TOKEN");
}

TEST_CASE("row width mismatch is rejected") {
    Board board;
    std::string error;
    CHECK_FALSE(parse("Board:\nwK . .\nbK .\n", board, error));
    CHECK(error == "ERROR ROW_WIDTH_MISMATCH");
}

TEST_CASE("canonical output normalizes irregular spacing") {
    Board board;
    std::string error;
    CHECK(parse("Board:\nwK    .   .\n", board, error));
    CHECK(printed(board) == "wK . .");
}

TEST_CASE("whitespace-only line inside the board block is skipped") {
    Board board;
    std::string error;
    CHECK(parse("Board:\nwK . .\n   \n. . .\n", board, error));
    CHECK(printed(board) == "wK . .\n. . .");
}

TEST_CASE("missing Board section yields an empty board") {
    Board board;
    std::string error;
    CHECK(parse("Commands:\nclick 0 0\n", board, error));
    CHECK(printed(board) == "");
}

TEST_CASE("inBounds respects board size") {
    Board board;
    std::string error;
    parse("Board:\nwK .\n. .\n", board, error);
    CHECK(board.inBounds(0, 0));
    CHECK(board.inBounds(1, 1));
    CHECK_FALSE(board.inBounds(-1, 0));
    CHECK_FALSE(board.inBounds(0, -1));
    CHECK_FALSE(board.inBounds(2, 0));
    CHECK_FALSE(board.inBounds(0, 2));
}

TEST_CASE("isEmpty distinguishes pieces from empty cells") {
    Board board;
    std::string error;
    parse("Board:\nwK .\n. .\n", board, error);
    CHECK_FALSE(board.isEmpty(0, 0));
    CHECK(board.isEmpty(0, 1));
}

TEST_CASE("isSameColor compares piece colors correctly") {
    Board board;
    std::string error;
    parse("Board:\nwK wQ\nbR .\n", board, error);
    CHECK(board.isSameColor(0, 0, 0, 1));
    CHECK_FALSE(board.isSameColor(0, 0, 1, 0));
    CHECK_FALSE(board.isSameColor(0, 0, 1, 1));
}

TEST_CASE("movePiece relocates a piece and clears its source") {
    Board board;
    std::string error;
    parse("Board:\nwK .\n. .\n", board, error);
    board.movePiece(0, 0, 1, 1);
    CHECK(board.isEmpty(0, 0));
    CHECK_FALSE(board.isEmpty(1, 1));
    CHECK(printed(board) == ". .\n. wK");
}

TEST_CASE("movePiece ignores out-of-bounds requests") {
    Board board;
    std::string error;
    parse("Board:\nwK .\n. .\n", board, error);
    board.movePiece(0, 0, 5, 5);
    CHECK_FALSE(board.isEmpty(0, 0));
}
