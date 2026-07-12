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

TEST_CASE("a second piece can move only after the first pending move has fully resolved") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . bK\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wK at (0,0)
    game.executeLine("click 50 150", out);  // move wK to (1,0), 1000ms
    game.executeLine("wait 1000", out);     // wK arrives: the board is free again
    game.executeLine("click 250 150", out); // select bK at (1,2)
    game.executeLine("click 150 150", out); // move bK to (1,1), 1000ms
    game.executeLine("wait 1000", out);
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

TEST_CASE("a piece cannot be redirected while it is already moving") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    game.executeLine("click 50 50", out);   // re-select wR (still shown at (0,0))
    game.executeLine("click 50 150", out);  // attempt redirect to (1,0): must be blocked
    game.executeLine("wait 2000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n. . .\n. . .\n");
}

TEST_CASE("a redirect attempt while moving does not cancel the original pending move") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    game.executeLine("click 50 50", out);   // re-select wR
    game.executeLine("click 50 150", out);  // blocked redirect attempt to (1,0)
    game.executeLine("wait 1000", out);     // half of the original duration: not yet arrived
    game.executeLine("print board", out);
    game.executeLine("wait 1000", out);     // now the original move fully arrives
    game.executeLine("print board", out);
    CHECK(out.str() ==
        "wR . .\n. . .\n. . .\n"
        ". . wR\n. . .\n. . .\n");
}

TEST_CASE("a piece can move again immediately after arriving, with no extra wait") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wK at (0,0)
    game.executeLine("click 150 50", out);  // move to (0,1), 1000ms
    game.executeLine("wait 1000", out);     // arrives
    game.executeLine("click 150 50", out);  // select wK at its new position (0,1)
    game.executeLine("click 250 50", out);  // move to (0,2), 1000ms: no extra cooldown needed
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wK\n. . .\n. . .\n");
}

TEST_CASE("redirect is blocked even when the new destination would otherwise be a legal move") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move to (0,2), 2000ms
    game.executeLine("click 50 50", out);   // re-select wR (still shown at origin)
    game.executeLine("click 150 50", out);  // attempt redirect to (0,1): otherwise legal, still blocked
    game.executeLine("wait 2000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("a piece of the opposite color cannot move while another piece is mid-transit") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\nbR . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    game.executeLine("click 50 250", out);  // select bR at (2,0)
    game.executeLine("click 250 250", out); // attempt to move bR to (2,2): blocked, board busy
    game.executeLine("wait 2000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n. . .\nbR . .\n");
}

TEST_CASE("a piece of the same color also cannot move while another piece is mid-transit") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\nwN . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    game.executeLine("click 50 250", out);  // select wN at (2,0)
    game.executeLine("click 250 150", out); // attempt to move wN to (1,2): blocked, board busy
    game.executeLine("wait 2000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n. . .\nwN . .\n");
}

TEST_CASE("capturing the enemy king ends the game") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move wR to (0,2): captures bK, 2000ms
    game.executeLine("wait 2000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("after the game is over, further click and wait commands are ignored") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\nbR . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move wR to (0,2): captures bK, 2000ms
    game.executeLine("wait 2000", out);     // game ends here: white wins
    game.executeLine("click 50 150", out);  // select bR at (1,0): must be ignored
    game.executeLine("click 150 150", out); // attempt to move bR to (1,1): must be ignored
    game.executeLine("wait 1000", out);     // must not resolve any move
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\nbR . .\n");
}

TEST_CASE("black capturing white's king reports black as the winner") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nbR . wK\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select bR at (0,0)
    game.executeLine("click 250 50", out);  // move bR to (0,2): captures wK, 2000ms
    game.executeLine("wait 2000", out);
    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::Black);
}

TEST_CASE("isGameOver and winner report correctly before and after white wins") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\n", error);
    std::ostringstream out;
    CHECK_FALSE(game.isGameOver());
    CHECK_FALSE(game.winner().has_value());

    game.executeLine("click 50 50", out);
    game.executeLine("click 250 50", out);
    game.executeLine("wait 2000", out);

    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::White);
}
