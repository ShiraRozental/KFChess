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

TEST_CASE("a pawn can move two cells from its start row and takes proportionally longer to arrive") {
    Game game;
    std::string error;
    // 5 rows: the start row is the bottom edge (row 4), and the destination
    // row 2 is not the promotion row, so this test isolates movement/timing
    // from the separate promotion behavior.
    game.loadBoard("Board:\n. . .\n. . .\n. . .\n. . .\nwP . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 450", out); // select wP at (4,0), its start row
    game.executeLine("click 50 250", out); // move to (2,0): Chebyshev distance 2, 2000ms
    game.executeLine("wait 1000", out);    // half the duration: not yet arrived
    game.executeLine("print board", out);
    game.executeLine("wait 1000", out);    // now the move fully arrives
    game.executeLine("print board", out);
    CHECK(out.str() ==
        ". . .\n. . .\n. . .\n. . .\nwP . .\n"
        ". . .\n. . .\nwP . .\n. . .\n. . .\n");
}

TEST_CASE("a pawn cannot move two cells from a row that is not its start row") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . .\n. . .\nwP . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 250", out); // select wP at (2,0): start row is (3), not (2)
    game.executeLine("click 50 50", out);  // attempt move to (0,0): illegal, no pending move
    game.executeLine("wait 5000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. . .\nwP . .\n. . .\n");
}

TEST_CASE("a white pawn becomes a queen when it reaches the last row") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwP . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 150", out); // select wP at (1,0)
    game.executeLine("click 50 50", out);  // move to (0,0): its promotion row, 1000ms
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wQ . .\n. . .\n");
}

TEST_CASE("a black pawn becomes a queen when it reaches the last row") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nbP . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select bP at (0,0)
    game.executeLine("click 50 150", out);  // move to (1,0): its promotion row, 1000ms
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\nbQ . .\n");
}

TEST_CASE("a pawn that captures diagonally into the last row also becomes a queen") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nbR . .\n. wP .\n", error);
    std::ostringstream out;
    game.executeLine("click 150 150", out); // select wP at (1,1)
    game.executeLine("click 50 50", out);   // capture bR at (0,0): its promotion row, 1000ms
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wQ . .\n. . .\n");
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

TEST_CASE("a jump with nothing arriving lands the piece back on its own square") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . .\n. wK .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("jump 150 150", out); // wK at (1,1) jumps in place
    game.executeLine("wait 1000", out);    // jump duration elapses, no arrival
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("an airborne piece captures an enemy that arrives at its cell before it lands") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK bR .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("jump 50 150", out);   // wK at (1,0) jumps, airborne until t=1000
    game.executeLine("click 150 150", out); // select bR at (1,1)
    game.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 1, arrives at t=1000
    game.executeLine("wait 1000", out);     // move and landing coincide: defense wins the tie
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK . .\n. . .\n");
}

TEST_CASE("jumping after a capture has already resolved does not undo it") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK bR .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("click 150 150", out); // select bR at (1,1)
    game.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 1, 1000ms
    game.executeLine("wait 1000", out);     // wK is captured before any jump is attempted
    game.executeLine("jump 50 150", out);   // too late: wK is already gone
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\nbR . .\n. . .\n");
}

TEST_CASE("an enemy that arrives after a jump has already landed captures normally") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . . .\nwK . . bR\n. . . .\n", error);
    std::ostringstream out;
    game.executeLine("jump 50 150", out);   // wK at (1,0) jumps, airborne until t=1000
    game.executeLine("wait 1000", out);     // jump lands normally, wK unprotected again
    game.executeLine("click 350 150", out); // select bR at (1,3)
    game.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 3, 3000ms
    game.executeLine("wait 3000", out);     // arrives at t=4000, long after wK landed
    game.executeLine("print board", out);
    CHECK(out.str() == ". . . .\nbR . . .\n. . . .\n");
}

TEST_CASE("a piece that is mid-transit cannot jump") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    std::ostringstream out;
    game.executeLine("click 50 50", out);   // select wR at (0,0)
    game.executeLine("click 250 50", out);  // move wR to (0,2): distance 2, 2000ms
    game.executeLine("wait 500", out);      // wR is still mid-transit
    game.executeLine("jump 50 50", out);    // blocked: a moving piece cannot jump
    game.executeLine("wait 1500", out);     // move arrives normally
    game.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("an airborne piece only captures an arriving enemy, not a same-color piece that could never legally move there") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK wR .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("jump 50 150", out);   // wK at (1,0) jumps
    game.executeLine("click 150 150", out); // select wR at (1,1), same color as wK
    game.executeLine("click 50 150", out);  // attempted move onto wK's cell is an illegal same-color capture
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK wR .\n. . .\n");
}

TEST_CASE("jumping on an empty cell does nothing") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK . .\n. . .\n", error);
    std::ostringstream out;
    game.executeLine("jump 250 150", out); // (1,2) is empty
    game.executeLine("wait 1000", out);
    game.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK . .\n. . .\n");
}

TEST_CASE("jumping at an out-of-bounds pixel is ignored") {
    Game game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n", error);
    std::ostringstream out;
    game.executeLine("jump -10 50", out);
    game.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n");
}

TEST_CASE("jumping an already-airborne piece is ignored and does not extend its protection") {
    Game game;
    std::string error;
    game.loadBoard("Board:\n. . . .\nwK . . bR\n. . . .\n", error);
    std::ostringstream out;
    game.executeLine("jump 50 150", out);   // wK at (1,0) jumps, airborne until t=1000
    game.executeLine("jump 50 150", out);   // duplicate attempt: ignored, does not reset the timer
    game.executeLine("wait 1000", out);     // original jump lands normally, wK unprotected again
    game.executeLine("click 350 150", out); // select bR at (1,3)
    game.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 3, 3000ms
    game.executeLine("wait 3000", out);     // arrives at t=4000, well after wK's original window
    game.executeLine("print board", out);
    CHECK(out.str() == ". . . .\nbR . . .\n. . . .\n");
}
