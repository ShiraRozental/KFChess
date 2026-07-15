#include "doctest/doctest.h"
#include "io/BoardParser.h"
#include "rules/LegalDestinations.h"
#include <sstream>

namespace {
    bool parse(const std::string& text, Board& board) {
        std::istringstream in(text);
        ParsedInput parsed;
        std::string error;
        bool ok = BoardParser::parse(in, parsed, error);
        board = std::move(parsed.board);
        return ok;
    }

    std::set<Position> destinationsAt(const Board& board, int row, int col) {
        return legalDestinationsFor(board, *board.pieceAt(row, col));
    }
}

TEST_CASE("rook slides along an empty rank and file until the board edge") {
    Board board;
    parse("Board:\nwR . .\n. . .\n. . .\n", board);
    CHECK(destinationsAt(board, 0, 0) ==
          std::set<Position>{{0, 1}, {0, 2}, {1, 0}, {2, 0}});
}

TEST_CASE("rook stops one cell before a friendly blocker and does not include it") {
    Board board;
    parse("Board:\n. wR wP .\n", board);
    CHECK(destinationsAt(board, 0, 1) == std::set<Position>{{0, 0}});
}

TEST_CASE("rook may capture an enemy blocker but not move past it") {
    Board board;
    parse("Board:\n. wR bP .\n", board);
    CHECK(destinationsAt(board, 0, 1) == std::set<Position>{{0, 0}, {0, 2}});
}

TEST_CASE("bishop reaches only diagonal cells, never straight ones") {
    Board board;
    parse("Board:\n. . .\n. wB .\n. . .\n", board);
    CHECK(destinationsAt(board, 1, 1) ==
          std::set<Position>{{0, 0}, {0, 2}, {2, 0}, {2, 2}});
}

TEST_CASE("queen's destinations are the union of a rook's and a bishop's from the same cell") {
    Board board;
    parse("Board:\n. . .\n. wQ .\n. . .\n", board);
    CHECK(destinationsAt(board, 1, 1) ==
          std::set<Position>{{0, 0}, {0, 1}, {0, 2},
                              {1, 0},         {1, 2},
                              {2, 0}, {2, 1}, {2, 2}});
}

TEST_CASE("knight jumps to its L-shaped cells regardless of adjacent friendly pieces") {
    Board board;
    parse("Board:\nwN . . .\n. . bP .\n. wP . .\n", board);
    CHECK(destinationsAt(board, 0, 0) == std::set<Position>{{1, 2}});
}

TEST_CASE("king reaches all eight surrounding cells, excluding a friendly one and capturing an enemy one") {
    Board board;
    parse("Board:\n. wP .\nbR wK .\n. . .\n", board);
    CHECK(destinationsAt(board, 1, 1) ==
          std::set<Position>{{0, 0}, {0, 2},
                              {1, 0},         {1, 2},
                              {2, 0}, {2, 1}, {2, 2}});
}

TEST_CASE("king in a corner is limited to the three in-bounds neighboring cells") {
    Board board;
    parse("Board:\nwK . .\n. . .\n. . .\n", board);
    CHECK(destinationsAt(board, 0, 0) ==
          std::set<Position>{{0, 1}, {1, 0}, {1, 1}});
}

TEST_CASE("white pawn advances one cell and captures diagonally only where an enemy stands") {
    Board board;
    parse("Board:\nbR . bR\n. wP .\n. . .\n", board);
    CHECK(destinationsAt(board, 1, 1) ==
          std::set<Position>{{0, 0}, {0, 1}, {0, 2}});
}

TEST_CASE("white pawn does not capture diagonally into empty cells") {
    Board board;
    parse("Board:\n. . .\n. wP .\n. . .\n", board);
    CHECK(destinationsAt(board, 1, 1) == std::set<Position>{{0, 1}});
}

TEST_CASE("white pawn from its start row includes both the single and double advance") {
    Board board;
    parse("Board:\n. . .\n. . .\n. . .\n. wP .\n", board);
    CHECK(destinationsAt(board, 3, 1) == std::set<Position>{{2, 1}, {1, 1}});
}

TEST_CASE("white pawn's double advance is excluded when the path or destination is blocked") {
    Board board;
    parse("Board:\n. . .\n. . .\n. wR .\n. wP .\n", board);
    CHECK(destinationsAt(board, 3, 1) == std::set<Position>{});
}

TEST_CASE("white pawn off its start row has no double-advance destination") {
    Board board;
    parse("Board:\n. . .\n. wP .\n. . .\n. . .\n", board);
    CHECK(destinationsAt(board, 1, 1) == std::set<Position>{{0, 1}});
}

TEST_CASE("black pawn advances toward increasing rows and captures diagonally there") {
    Board board;
    parse("Board:\n. . .\n. bP .\nwR . wR\n", board);
    CHECK(destinationsAt(board, 1, 1) ==
          std::set<Position>{{2, 0}, {2, 1}, {2, 2}});
}

TEST_CASE("black pawn from its start row includes both the single and double advance") {
    Board board;
    parse("Board:\n. bP .\n. . .\n. . .\n. . .\n", board);
    CHECK(destinationsAt(board, 0, 1) == std::set<Position>{{1, 1}, {2, 1}});
}
