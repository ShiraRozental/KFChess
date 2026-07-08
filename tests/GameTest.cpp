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
