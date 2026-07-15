#include "doctest/doctest.h"
#include "io/BoardParser.h"
#include <sstream>

namespace {
    bool parse(const std::string& text, ParsedInput& out, std::string& error) {
        std::istringstream in(text);
        return BoardParser::parse(in, out, error);
    }
}

TEST_CASE("valid board parses and infers dimensions") {
    ParsedInput parsed;
    std::string error;
    CHECK(parse("Board:\nwK . .\n. . .\n. . .\n", parsed, error));
    CHECK(parsed.board.rowCount() == 3);
    CHECK(parsed.board.colCount() == 3);
}

TEST_CASE("unknown token is rejected") {
    ParsedInput parsed;
    std::string error;
    CHECK_FALSE(parse("Board:\nwK xZ\n. .\n", parsed, error));
    CHECK(error == "ERROR UNKNOWN_TOKEN");
}

TEST_CASE("row width mismatch is rejected") {
    ParsedInput parsed;
    std::string error;
    CHECK_FALSE(parse("Board:\nwK . .\nbK .\n", parsed, error));
    CHECK(error == "ERROR ROW_WIDTH_MISMATCH");
}

TEST_CASE("whitespace-only line inside the board block is skipped") {
    ParsedInput parsed;
    std::string error;
    CHECK(parse("Board:\nwK . .\n   \n. . .\n", parsed, error));
    CHECK(parsed.board.rowCount() == 2);
}

TEST_CASE("missing Board section yields an empty board") {
    ParsedInput parsed;
    std::string error;
    CHECK(parse("Commands:\nclick 0 0\n", parsed, error));
    CHECK(parsed.board.rowCount() == 0);
}

TEST_CASE("parsed pieces get sequential ids in row-major scan order") {
    ParsedInput parsed;
    std::string error;
    CHECK(parse("Board:\nwK . bQ\n", parsed, error));
    REQUIRE(parsed.board.pieceAt(0, 0) != nullptr);
    REQUIRE(parsed.board.pieceAt(0, 2) != nullptr);
    CHECK(parsed.board.pieceAt(0, 0)->id() == 0);
    CHECK(parsed.board.pieceAt(0, 2)->id() == 1);
}

TEST_CASE("board text and command lines are split correctly") {
    ParsedInput parsed;
    std::string error;
    CHECK(parse("Board:\nwK .\n. .\nCommands:\nclick 0 0\nprint board\n", parsed, error));
    CHECK(parsed.board.rowCount() == 2);
    REQUIRE(parsed.commands.size() == 2);
    CHECK(parsed.commands[0] == "click 0 0");
    CHECK(parsed.commands[1] == "print board");
}

TEST_CASE("blank and whitespace-only command lines are skipped") {
    ParsedInput parsed;
    std::string error;
    CHECK(parse("Board:\nwK .\nCommands:\n   \nclick 0 0\n", parsed, error));
    REQUIRE(parsed.commands.size() == 1);
    CHECK(parsed.commands[0] == "click 0 0");
}
