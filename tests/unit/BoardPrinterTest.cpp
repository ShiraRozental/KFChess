#include "doctest/doctest.h"
#include "io/BoardPrinter.h"
#include "io/BoardParser.h"
#include <sstream>

namespace {
    Board parsedBoard(const std::string& text) {
        std::istringstream in(text);
        ParsedInput parsed;
        std::string error;
        BoardParser::parse(in, parsed, error);
        return parsed.board;
    }

    std::string written(const Board& board) {
        GameSnapshot snapshot(board, false, std::nullopt);
        std::ostringstream out;
        printBoard(snapshot, out);
        return out.str();
    }
}

TEST_CASE("printBoard renders a parsed board back to its canonical text form") {
    CHECK(written(parsedBoard("Board:\nwK . .\n. . .\n. . .\n")) == "wK . .\n. . .\n. . .");
}

TEST_CASE("printBoard normalizes irregular spacing") {
    CHECK(written(parsedBoard("Board:\nwK    .   .\n")) == "wK . .");
}

TEST_CASE("printBoard reflects a promotion made directly on the board") {
    Board board = parsedBoard("Board:\nwP .\n. .\n");
    board.promote(0, 0, PieceType::Queen);
    CHECK(written(board) == "wQ .\n. .");
}

TEST_CASE("printBoard on an empty board prints nothing") {
    CHECK(written(parsedBoard("Commands:\nclick 0 0\n")) == "");
}

TEST_CASE("printParseError writes the error message followed by a newline") {
    std::ostringstream out;
    printParseError("ERROR UNKNOWN_TOKEN", out);
    CHECK(out.str() == "ERROR UNKNOWN_TOKEN\n");
}
