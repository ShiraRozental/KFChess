#include "doctest/doctest.h"
#include "engine/GameEngine.h"
#include "texttests/TextTestRunner.h"
#include "rules/RuleEngine.h"
#include <sstream>

TEST_CASE("click selects then moves a piece") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 150 150", out);
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("click just outside the board with a negative pixel is ignored") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\nbR . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click -10 150", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wK . .\nbR . .\n. . .\n");
}

TEST_CASE("clicking an empty cell without a selection is ignored") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 250 250", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("clicking a friendly piece as the second click is rejected, not reselected") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . wK\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // second click on wK (same color): request rejected, selection cleared (no reselect)
    runner.executeLine("click 250 150", out); // fresh first click on an empty cell: no-op
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wR . wK\n. . .\n");
}

TEST_CASE("malformed click arguments are ignored") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK .\n. .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click abc def", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wK .\n. .\n");
}

TEST_CASE("loadBoard surfaces parsing errors") {
    GameEngine game;
    std::string error;
    CHECK_FALSE(game.loadBoard("Board:\nwK xZ\n", error));
    CHECK(error == "ERROR UNKNOWN_TOKEN");
}

TEST_CASE("clicking a rook blocked by a piece in its path does not move it") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR bP .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 250 50", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wR bP .\n");
}

TEST_CASE("clicking a rook with a clear path captures the enemy piece at the destination") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . bP\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 250 50", out);
    runner.executeLine("wait 2000", out); // 2-cell move: 2 x kMoveDurationPerCellMs
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("knight jumps over surrounding pieces to reach its destination") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwN bP bP\nbP bP .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 250 150", out);
    runner.executeLine("wait 2000", out); // Chebyshev distance 2: 2 x kMoveDurationPerCellMs
    runner.executeLine("print board", out);
    CHECK(out.str() == ". bP bP\nbP bP wN\n");
}

TEST_CASE("a two-cell move needs two cells' worth of wait time to arrive") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 250 50", out);
    runner.executeLine("wait 1000", out);     // halfway: shown at the intermediate cell it just entered
    runner.executeLine("print board", out);
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". wR .\n. . wR\n");
}

TEST_CASE("a move does not appear on the board before its arrival time") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 150 150", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("a move does not appear after a wait shorter than the move duration") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 150 150", out);
    runner.executeLine("wait 500", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("a move appears at its destination after waiting long enough") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 150 150", out);
    runner.executeLine("wait 5000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("a move appears exactly when the wait matches the move duration") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 150 150", out);
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("a second piece can move only after the first pending move has fully resolved") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . bK\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wK at (0,0)
    runner.executeLine("click 50 150", out);  // move wK to (1,0), 1000ms
    runner.executeLine("wait 1000", out);     // wK arrives: the board is free again
    runner.executeLine("click 250 150", out); // select bK at (1,2)
    runner.executeLine("click 150 150", out); // move bK to (1,1), 1000ms
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK bK .\n. . .\n");
}

TEST_CASE("an illegal click does not schedule a pending move") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR bP .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);
    runner.executeLine("click 250 50", out);
    runner.executeLine("wait 5000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wR bP .\n");
}

TEST_CASE("a piece cannot be redirected while it is already moving") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    runner.executeLine("click 50 50", out);   // re-select wR (still shown at (0,0))
    runner.executeLine("click 50 150", out);  // attempt redirect to (1,0): must be blocked
    runner.executeLine("wait 2000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n. . .\n. . .\n");
}

TEST_CASE("a redirect attempt while moving does not cancel the original pending move") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    runner.executeLine("click 50 50", out);   // re-select wR
    runner.executeLine("click 50 150", out);  // blocked redirect attempt to (1,0)
    runner.executeLine("wait 1000", out);     // half of the original duration: shown at the intermediate cell
    runner.executeLine("print board", out);
    runner.executeLine("wait 1000", out);     // now the original move fully arrives
    runner.executeLine("print board", out);
    CHECK(out.str() ==
        ". wR .\n. . .\n. . .\n"
        ". . wR\n. . .\n. . .\n");
}

TEST_CASE("a piece can move again immediately after arriving, with no extra wait") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wK at (0,0)
    runner.executeLine("click 150 50", out);  // move to (0,1), 1000ms
    runner.executeLine("wait 1000", out);     // arrives
    runner.executeLine("click 150 50", out);  // select wK at its new position (0,1)
    runner.executeLine("click 250 50", out);  // move to (0,2), 1000ms: no extra cooldown needed
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wK\n. . .\n. . .\n");
}

TEST_CASE("redirect is blocked even when the new destination would otherwise be a legal move") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move to (0,2), 2000ms
    runner.executeLine("click 50 50", out);   // re-select wR (still shown at origin)
    runner.executeLine("click 150 50", out);  // attempt redirect to (0,1): otherwise legal, still blocked
    runner.executeLine("wait 2000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("a piece of the opposite color can move concurrently while another piece is mid-transit") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\nbR . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    runner.executeLine("click 50 250", out);  // select bR at (2,0)
    runner.executeLine("click 250 250", out); // bR to (2,2), 2000ms — a different piece, runs alongside wR
    runner.executeLine("wait 2000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n. . .\n. . bR\n");
}

TEST_CASE("a piece of the same color can also move concurrently while another piece is mid-transit") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\nwN . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move wR to (0,2), 2000ms
    runner.executeLine("click 50 250", out);  // select wN at (2,0)
    runner.executeLine("click 250 150", out); // wN to (1,2), Chebyshev distance 2: 2000ms — runs alongside wR
    runner.executeLine("wait 2000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n. . wN\n. . .\n");
}

TEST_CASE("capturing the enemy king ends the game") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move wR to (0,2): captures bK, 2000ms
    runner.executeLine("wait 2000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("after the game is over, further click and wait commands are ignored") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\nbR . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move wR to (0,2): captures bK, 2000ms
    runner.executeLine("wait 2000", out);     // game ends here: white wins
    runner.executeLine("click 50 150", out);  // select bR at (1,0): must be ignored
    runner.executeLine("click 150 150", out); // attempt to move bR to (1,1): must be ignored
    runner.executeLine("wait 1000", out);     // must not resolve any move
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\nbR . .\n");
}

TEST_CASE("black capturing white's king reports black as the winner") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nbR . wK\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select bR at (0,0)
    runner.executeLine("click 250 50", out);  // move bR to (0,2): captures wK, 2000ms
    runner.executeLine("wait 2000", out);
    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::Black);
}

TEST_CASE("a pawn can move two cells from its start row and takes proportionally longer to arrive") {
    GameEngine game;
    std::string error;
    // 5 rows: the start row is the bottom edge (row 4), and the destination
    // row 2 is not the promotion row, so this test isolates movement/timing
    // from the separate promotion behavior.
    game.loadBoard("Board:\n. . .\n. . .\n. . .\n. . .\nwP . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 450", out); // select wP at (4,0), its start row
    runner.executeLine("click 50 250", out); // move to (2,0): Chebyshev distance 2, 2000ms
    runner.executeLine("wait 1000", out);    // half the duration: shown at the intermediate cell
    runner.executeLine("print board", out);
    runner.executeLine("wait 1000", out);    // now the move fully arrives
    runner.executeLine("print board", out);
    CHECK(out.str() ==
        ". . .\n. . .\n. . .\nwP . .\n. . .\n"
        ". . .\n. . .\nwP . .\n. . .\n. . .\n");
}

TEST_CASE("a pawn cannot move two cells from a row that is not its start row") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\n. . .\nwP . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 250", out); // select wP at (2,0): start row is (3), not (2)
    runner.executeLine("click 50 50", out);  // attempt move to (0,0): illegal, no pending move
    runner.executeLine("wait 5000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. . .\nwP . .\n. . .\n");
}

TEST_CASE("a white pawn becomes a queen when it reaches the last row") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwP . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 150", out); // select wP at (1,0)
    runner.executeLine("click 50 50", out);  // move to (0,0): its promotion row, 1000ms
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wQ . .\n. . .\n");
}

TEST_CASE("a black pawn becomes a queen when it reaches the last row") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nbP . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select bP at (0,0)
    runner.executeLine("click 50 150", out);  // move to (1,0): its promotion row, 1000ms
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\nbQ . .\n");
}

TEST_CASE("a pawn that captures diagonally into the last row also becomes a queen") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nbR . .\n. wP .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 150 150", out); // select wP at (1,1)
    runner.executeLine("click 50 50", out);   // capture bR at (0,0): its promotion row, 1000ms
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wQ . .\n. . .\n");
}

TEST_CASE("isGameOver and winner report correctly before and after white wins") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    CHECK_FALSE(game.isGameOver());
    CHECK_FALSE(game.winner().has_value());

    runner.executeLine("click 50 50", out);
    runner.executeLine("click 250 50", out);
    runner.executeLine("wait 2000", out);

    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::White);
}

TEST_CASE("a jump with nothing arriving lands the piece back on its own square") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\n. wK .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("jump 150 150", out); // wK at (1,1) jumps in place
    runner.executeLine("wait 1000", out);    // jump duration elapses, no arrival
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("an airborne piece captures an enemy that arrives at its cell before it lands") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK bR .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("jump 50 150", out);   // wK at (1,0) jumps, airborne until t=1000
    runner.executeLine("click 150 150", out); // select bR at (1,1)
    runner.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 1, arrives at t=1000
    runner.executeLine("wait 1000", out);     // move and landing coincide: defense wins the tie
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK . .\n. . .\n");
}

TEST_CASE("jumping after a capture has already resolved does not undo it") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK bR .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 150 150", out); // select bR at (1,1)
    runner.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 1, 1000ms
    runner.executeLine("wait 1000", out);     // wK is captured before any jump is attempted
    runner.executeLine("jump 50 150", out);   // too late: wK is already gone
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\nbR . .\n. . .\n");
}

TEST_CASE("an enemy that arrives after a jump has already landed captures normally") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . . .\nwK . . bR\n. . . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("jump 50 150", out);   // wK at (1,0) jumps, airborne until t=1000
    runner.executeLine("wait 1000", out);     // jump lands normally, wK unprotected again
    runner.executeLine("click 350 150", out); // select bR at (1,3)
    runner.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 3, 3000ms
    runner.executeLine("wait 3000", out);     // arrives at t=4000, long after wK landed
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . . .\nbR . . .\n. . . .\n");
}

TEST_CASE("a piece that is mid-transit cannot jump") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("click 50 50", out);   // select wR at (0,0)
    runner.executeLine("click 250 50", out);  // move wR to (0,2): distance 2, 2000ms
    runner.executeLine("wait 500", out);      // wR is still mid-transit
    runner.executeLine("jump 50 50", out);    // blocked: a moving piece cannot jump
    runner.executeLine("wait 1500", out);     // move arrives normally
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("an airborne piece only captures an arriving enemy, not a same-color piece that could never legally move there") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK wR .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("jump 50 150", out);   // wK at (1,0) jumps
    runner.executeLine("click 150 150", out); // select wR at (1,1), same color as wK
    runner.executeLine("click 50 150", out);  // attempted move onto wK's cell is an illegal same-color capture
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK wR .\n. . .\n");
}

TEST_CASE("jumping on an empty cell does nothing") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK . .\n. . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("jump 250 150", out); // (1,2) is empty
    runner.executeLine("wait 1000", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . .\nwK . .\n. . .\n");
}

TEST_CASE("jumping at an out-of-bounds pixel is ignored") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("jump -10 50", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wK . .\n");
}

TEST_CASE("jumping an already-airborne piece is ignored and does not extend its protection") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . . .\nwK . . bR\n. . . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    runner.executeLine("jump 50 150", out);   // wK at (1,0) jumps, airborne until t=1000
    runner.executeLine("jump 50 150", out);   // duplicate attempt: ignored, does not reset the timer
    runner.executeLine("wait 1000", out);     // original jump lands normally, wK unprotected again
    runner.executeLine("click 350 150", out); // select bR at (1,3)
    runner.executeLine("click 50 150", out);  // bR moves onto wK's cell: distance 3, 3000ms
    runner.executeLine("wait 3000", out);     // arrives at t=4000, well after wK's original window
    runner.executeLine("print board", out);
    CHECK(out.str() == ". . . .\nbR . . .\n. . . .\n");
}

TEST_CASE("a king intercepted by an enemy's active jump loses the game immediately") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK bP .\n", error);
    game.requestJump(Position{0, 1});                      // bP jumps, airborne until t=1000
    game.requestMove(Position{0, 0}, Position{0, 1});       // wK attacks bP's cell, arrives at t=1000
    game.wait(1000);

    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::Black);
    CHECK(game.hasPieceAt(Position{0, 1}));                 // bP survives
    CHECK_FALSE(game.hasPieceAt(Position{0, 0}));           // wK never lands anywhere
}

// The tests below exercise GameEngine's public API directly, in Position
// terms, with no pixels and no TextTestRunner/Controller involved.

TEST_CASE("requestMove accepts a legal move and returns ok") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 2});
    CHECK(result.is_accepted);
    CHECK(result.reason == MoveResultReason::Ok);
}

TEST_CASE("requestMove rejects an illegal move and returns the rule-level reason") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . wP\n", error);
    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 2});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveRejectionReason::FriendlyDestination);
}

TEST_CASE("requestMove rejects with game_over once the game has ended") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\n", error);
    game.requestMove(Position{0, 0}, Position{0, 2}); // captures bK, 2000ms
    game.wait(2000);
    REQUIRE(game.isGameOver());

    MoveResult result = game.requestMove(Position{0, 2}, Position{0, 1});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveResultReason::GameOver);
}

TEST_CASE("requestMove accepts a different piece while another move is in flight") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n. . .\nbR . .\n", error);
    game.requestMove(Position{0, 0}, Position{0, 2}); // wR: 2000ms in flight

    MoveResult result = game.requestMove(Position{2, 0}, Position{2, 2});
    CHECK(result.is_accepted);
    CHECK(result.reason == MoveResultReason::Ok);
}

TEST_CASE("requestMove rejects with motion_in_progress for the same piece that is already in flight") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    game.requestMove(Position{0, 0}, Position{0, 2}); // wR: 2000ms in flight

    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 1});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveResultReason::MotionInProgress);
}

TEST_CASE("two same-color paths crossing mid-flight: the later one stops short, end-to-end through requestMove/wait") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . wR . .\n. . . . .\n. . . . wR\n. . . . .\n. . . . .\n", error);

    game.requestMove(Position{2, 4}, Position{2, 0}); // starts at t=0, passes (2,2) at t=2000
    game.wait(500);
    game.requestMove(Position{0, 2}, Position{4, 2}); // starts at t=500, passes (2,2) at t=2500 — later

    game.wait(2000); // clock now 2500: the crossing resolves
    CHECK(game.hasPieceAt(Position{1, 2}));           // stopped one cell short of (2,2)
    CHECK_FALSE(game.hasPieceAt(Position{2, 2}));
    CHECK_FALSE(game.hasPieceAt(Position{4, 2}));      // never reached its original destination

    game.wait(1500); // clock now 4000: the first rook's own move completes
    CHECK(game.hasPieceAt(Position{2, 0}));
}

TEST_CASE("a mover arriving at a cell a friend already settled bounces back instead of overwriting it") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR .\n. .\n. wR\n", error);

    // The first mover settles (and leaves the arbiter's tracking entirely)
    // in its own wait() call, before the second even starts — so the
    // second's collision is against settled Board state, not an in-flight
    // conflict the arbiter itself would already have resolved.
    game.requestMove(Position{0, 0}, Position{0, 1});
    game.wait(1000);
    REQUIRE(game.hasPieceAt(Position{0, 1}));

    game.requestMove(Position{2, 1}, Position{0, 1}); // 2000ms
    game.wait(2000);

    CHECK(game.hasPieceAt(Position{0, 1}));  // the first mover, untouched
    CHECK(game.hasPieceAt(Position{2, 1}));  // the second mover, bounced back home
}

TEST_CASE("a king that loses a mid-flight collision (not a jump) ends the game") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK .\n. .\n. .\n. bR\n", error);

    game.requestMove(Position{0, 0}, Position{0, 1}); // wK: arrives (0,1) at t=1000
    game.requestMove(Position{3, 1}, Position{0, 1}); // bR: arrives (0,1) at t=3000 — later, wK loses
    game.wait(3000);

    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::Black);
    CHECK_FALSE(game.hasPieceAt(Position{0, 1})); // wK never lands
    CHECK(game.hasPieceAt(Position{3, 1}));        // bR's own move is voided once the game is over
}

TEST_CASE("requestJump starts a motion that keeps the piece airborne") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK bR .\n. . .\n", error);
    game.requestJump(Position{1, 0}); // wK jumps, airborne until t=1000

    // bR moving onto wK's cell is intercepted while the jump is active.
    game.requestMove(Position{1, 1}, Position{1, 0});
    game.wait(1000);

    CHECK(game.hasPieceAt(Position{1, 0}));
    CHECK_FALSE(game.hasPieceAt(Position{1, 1}));
}

TEST_CASE("a printed board still shows a mid-flight piece at its source until it arrives") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    game.requestMove(Position{0, 0}, Position{0, 2}); // 2000ms in flight
    runner.executeLine("wait 500", out);              // mid-flight
    runner.executeLine("print board", out);
    CHECK(out.str() == "wR . .\n");
}

TEST_CASE("a mid-flight piece's source cell reads as empty to hasPieceAt") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . .\n", error);
    game.requestMove(Position{0, 0}, Position{0, 2}); // 2000ms in flight
    game.wait(500);

    // The piece has left the board for the duration of its flight — it can
    // no longer be selected or commanded, which is exactly the movement
    // lock: a piece mid-walk cannot be told to go somewhere new.
    CHECK_FALSE(game.hasPieceAt(Position{0, 0}));
}

TEST_CASE("requestMove on a mid-jump piece is rejected with motion_in_progress") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n", error);
    game.requestJump(Position{0, 0}); // airborne until t=1000

    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 1});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveResultReason::MotionInProgress);
}

TEST_CASE("a king mid-jump never disappears from the printed board") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . bR\n", error);
    TextTestRunner runner(game);
    std::ostringstream out;
    game.requestJump(Position{0, 0}); // airborne until t=1000
    runner.executeLine("wait 500", out);
    runner.executeLine("print board", out);
    CHECK(out.str() == "wK . bR\n");
}

TEST_CASE("hasPieceAt distinguishes occupied cells from empty ones") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n", error);
    CHECK(game.hasPieceAt(Position{0, 0}));
    CHECK_FALSE(game.hasPieceAt(Position{0, 1}));
}

TEST_CASE("snapshot reflects the current board and game-over state") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . bK\n", error);

    GameSnapshot before = game.snapshot();
    CHECK_FALSE(before.isGameOver());
    CHECK(before.board().pieceAt(0, 2) != nullptr);

    game.requestMove(Position{0, 0}, Position{0, 2}); // captures bK, 2000ms
    game.wait(2000);

    GameSnapshot after = game.snapshot();
    CHECK(after.isGameOver());
    REQUIRE(after.winner().has_value());
    CHECK(*after.winner() == PieceColor::White);
}
