#include "doctest/doctest.h"
#include "io/BoardTextFormat.h"
#include <sstream>

namespace {
    bool parse(const std::string& text, Board& board, std::string& error) {
        std::istringstream in(text);
        return BoardTextFormat::parse(in, board, error);
    }

    std::string written(const Board& board) {
        std::ostringstream out;
        BoardTextFormat::write(board, out);
        return out.str();
    }
}

TEST_CASE("valid board parses and infers dimensions") {
    Board board;
    std::string error;
    CHECK(parse("Board:\nwK . .\n. . .\n. . .\n", board, error));
    CHECK(board.rowCount() == 3);
    CHECK(board.colCount() == 3);
    CHECK(written(board) == "wK . .\n. . .\n. . .");
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
    CHECK(written(board) == "wK . .");
}

TEST_CASE("whitespace-only line inside the board block is skipped") {
    Board board;
    std::string error;
    CHECK(parse("Board:\nwK . .\n   \n. . .\n", board, error));
    CHECK(written(board) == "wK . .\n. . .");
}

TEST_CASE("missing Board section yields an empty board") {
    Board board;
    std::string error;
    CHECK(parse("Commands:\nclick 0 0\n", board, error));
    CHECK(written(board) == "");
    CHECK(board.rowCount() == 0);
}

TEST_CASE("parsed pieces get sequential ids in row-major scan order") {
    Board board;
    std::string error;
    CHECK(parse("Board:\nwK . bQ\n", board, error));
    REQUIRE(board.pieceAt(0, 0) != nullptr);
    REQUIRE(board.pieceAt(0, 2) != nullptr);
    CHECK(board.pieceAt(0, 0)->id() == 0);
    CHECK(board.pieceAt(0, 2)->id() == 1);
}

TEST_CASE("write reflects a promotion made directly on the parsed board") {
    Board board;
    std::string error;
    CHECK(parse("Board:\nwP .\n. .\n", board, error));
    board.promote(0, 0, PieceType::Queen);
    CHECK(written(board) == "wQ .\n. .");
}
