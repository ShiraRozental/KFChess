#include "doctest/doctest.h"
#include "Game.h"
#include <sstream>

TEST_CASE("click selects then moves a piece") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 150 150", out);
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("click just outside the board with a negative pixel is ignored") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\nbR . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click -10 150", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wK . .\nbR . .\n. . .\n");
}

TEST_CASE("clicking an empty cell without a selection is ignored") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 250 250", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("clicking another friendly piece replaces the selection") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . wK\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 250 50", out);
    game.executeLine("click 250 150", out);
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wR . .\n. . wK\n");
}

TEST_CASE("malformed click arguments are ignored") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK .\n. .\n", error);
    std::ostringstream out;
    game.executeLine("click abc def", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wK .\n. .\n");
}

TEST_CASE("loadBoard surfaces parsing errors") {
    Game game;
    std::string error;
    CHECK_FALSE(game.loadBoard("Board:\nwK xZ\n", error));
    CHECK(error == "ERROR UNKNOWN_TOKEN");
}

TEST_CASE("clicking a rook blocked by a piece in its path does not move it") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR bP .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 250 50", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wR bP .\n");
}

TEST_CASE("clicking a rook with a clear path captures the enemy piece at the destination") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . bP\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 250 50", out);
    game.executeLine("wait 2000", out); // 2-cell move: 2 x kMoveDurationPerCellMs
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("knight jumps over surrounding pieces to reach its destination") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwN bP bP\nbP bP .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 250 150", out);
    game.executeLine("wait 2000", out); // Chebyshev distance 2: 2 x kMoveDurationPerCellMs
    game.executeLine("print board", out);
    CHECK(out.str() == ". bP bP\nbP bP wN\n");
}

TEST_CASE("a two-cell move needs two cells' worth of wait time to arrive") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 250 50", out);
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wR . .\n. . wR\n");
}

TEST_CASE("a move does not appear on the board before its arrival time") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 150 150", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("a move does not appear after a wait shorter than the move duration") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 150 150", out);
    game.executeLine("wait 500", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("a move appears at its destination after waiting long enough") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 150 150", out);
    game.executeLine("wait 5000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("a move appears exactly when the wait matches the move duration") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 150 150", out);
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("two independent pending moves both resolve after enough wait") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . bK\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wK at (0,0)
    game.executeLine("click 50 150", out);  // move wK to (1,0)
    game.executeLine("click 250 150", out); // select bK at (1,2)
    game.executeLine("click 150 150", out); // move bK to (1,1)
    game.executeLine("wait 5000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK bK .\n. . .\n");
}

TEST_CASE("an illegal click does not schedule a pending move") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR bP .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);
    game.executeLine("click 250 50", out);
    game.executeLine("wait 5000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wR bP .\n");
}
