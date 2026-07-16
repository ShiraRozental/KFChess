#include "doctest/doctest.h"
#include "engine/GameEngine.h"
#include "io/BoardParser.h"
#include "input/BoardMapper.h"
#include "input/Controller.h"
#include "texttests/ScriptRunner.h"
#include "rules/RuleEngine.h"
#include <sstream>

namespace {
    constexpr int kCellSizePixels = 100;

    GameEngine makeGame(const std::string& boardText) {
        std::istringstream in(boardText);
        ParsedInput parsed;
        std::string error;
        BoardParser::parse(in, parsed, error);
        return GameEngine(std::move(parsed.board));
    }

    Controller makeController(GameEngine& game) {
        const Board& board = game.snapshot().board();
        return Controller(game, BoardMapper(board.rowCount(), board.colCount(), kCellSizePixels));
    }
}

TEST_CASE("click selects then moves a piece") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 150 150");
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("click just outside the board with a negative pixel is ignored") {
    GameEngine game = makeGame("Board:\nwK . .\nbR . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click -10 150");
    runLine(runner, "print board");
    CHECK(out.str() == "wK . .\nbR . .\n. . .\n");
}

TEST_CASE("clicking an empty cell without a selection is ignored") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 250 250");
    runLine(runner, "print board");
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("clicking a friendly piece as the second click is rejected, not reselected") {
    GameEngine game = makeGame("Board:\nwR . wK\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // second click on wK (same color): request rejected, selection cleared (no reselect)
    runLine(runner, "click 250 150"); // fresh first click on an empty cell: no-op
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == "wR . wK\n. . .\n");
}

TEST_CASE("malformed click arguments are ignored") {
    GameEngine game = makeGame("Board:\nwK .\n. .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click abc def");
    runLine(runner, "print board");
    CHECK(out.str() == "wK .\n. .\n");
}

TEST_CASE("clicking a rook blocked by a piece in its path does not move it") {
    GameEngine game = makeGame("Board:\nwR bP .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 250 50");
    runLine(runner, "print board");
    CHECK(out.str() == "wR bP .\n");
}

TEST_CASE("clicking a rook with a clear path captures the enemy piece at the destination") {
    GameEngine game = makeGame("Board:\nwR . bP\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 250 50");
    runLine(runner, "wait 2000"); // 2-cell move: 2 x kMoveDurationPerCellMs
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("knight jumps over surrounding pieces to reach its destination") {
    GameEngine game = makeGame("Board:\nwN bP bP\nbP bP .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 250 150");
    runLine(runner, "wait 2000"); // Chebyshev distance 2: 2 x kMoveDurationPerCellMs
    runLine(runner, "print board");
    CHECK(out.str() == ". bP bP\nbP bP wN\n");
}

TEST_CASE("a two-cell move needs two cells' worth of wait time to arrive") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 250 50");
    runLine(runner, "wait 1000");     // halfway: shown at the intermediate cell it just entered
    runLine(runner, "print board");
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". wR .\n. . wR\n");
}

TEST_CASE("a move does not appear on the board before its arrival time") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 150 150");
    runLine(runner, "print board");
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("a move does not appear after a wait shorter than the move duration") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 150 150");
    runLine(runner, "wait 500");
    runLine(runner, "print board");
    CHECK(out.str() == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("a move appears at its destination after waiting long enough") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 150 150");
    runLine(runner, "wait 5000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("a move appears exactly when the wait matches the move duration") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 150 150");
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("a second piece can move only after the first pending move has fully resolved") {
    GameEngine game = makeGame("Board:\nwK . .\n. . bK\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wK at (0,0)
    runLine(runner, "click 50 150");  // move wK to (1,0), 1000ms
    runLine(runner, "wait 1000");     // wK arrives: the board is free again
    runLine(runner, "click 250 150"); // select bK at (1,2)
    runLine(runner, "click 150 150"); // move bK to (1,1), 1000ms
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\nwK bK .\n. . .\n");
}

TEST_CASE("an illegal click does not schedule a pending move") {
    GameEngine game = makeGame("Board:\nwR bP .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 250 50");
    runLine(runner, "wait 5000");
    runLine(runner, "print board");
    CHECK(out.str() == "wR bP .\n");
}

TEST_CASE("a piece cannot be redirected while it is already moving") {
    GameEngine game = makeGame("Board:\nwR . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move wR to (0,2), 2000ms
    runLine(runner, "click 50 50");   // re-select wR (still shown at (0,0))
    runLine(runner, "click 50 150");  // attempt redirect to (1,0): must be blocked
    runLine(runner, "wait 2000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\n. . .\n. . .\n");
}

TEST_CASE("a redirect attempt while moving does not cancel the original pending move") {
    GameEngine game = makeGame("Board:\nwR . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move wR to (0,2), 2000ms
    runLine(runner, "click 50 50");   // re-select wR
    runLine(runner, "click 50 150");  // blocked redirect attempt to (1,0)
    runLine(runner, "wait 1000");     // half of the original duration: shown at the intermediate cell
    runLine(runner, "print board");
    runLine(runner, "wait 1000");     // now the original move fully arrives
    runLine(runner, "print board");
    CHECK(out.str() ==
        ". wR .\n. . .\n. . .\n"
        ". . wR\n. . .\n. . .\n");
}

TEST_CASE("a piece cannot move again immediately after arriving, until its cooldown elapses") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 150 50");
    runLine(runner, "wait 1000");
    runLine(runner, "click 150 50");
    runLine(runner, "click 250 50");
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". wK .\n. . .\n. . .\n");
}

TEST_CASE("a piece can move again once its cooldown has fully elapsed") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");
    runLine(runner, "click 150 50");
    runLine(runner, "wait 1000");
    runLine(runner, "wait 1000");
    runLine(runner, "click 150 50");
    runLine(runner, "click 250 50");
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . wK\n. . .\n. . .\n");
}

TEST_CASE("a piece that just landed from a move cannot jump for defense until its cooldown elapses") {
    GameEngine game = makeGame("Board:\nwK . .\n. bR .\n");
    game.requestMove(Position{0, 0}, Position{0, 1});
    game.wait(1000);

    game.requestJump(Position{0, 1});
    game.requestMove(Position{1, 1}, Position{0, 1});
    game.wait(1000);

    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::Black);
}

TEST_CASE("a piece cannot move immediately after a jump lands, until its short cooldown elapses") {
    GameEngine game = makeGame("Board:\n. . .\nwK . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 50 150");   // wK at (1,0) jumps, airborne until t=1000
    runLine(runner, "wait 1000");     // jump lands, jump cooldown starts
    runLine(runner, "click 50 150");  // attempt to select and move it right away
    runLine(runner, "click 150 150");
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\nwK . .\n. . .\n");
}

TEST_CASE("a piece can move again once its jump cooldown has fully elapsed") {
    GameEngine game = makeGame("Board:\n. . .\nwK . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 50 150");   // wK at (1,0) jumps, airborne until t=1000
    runLine(runner, "wait 1000");     // jump lands, jump cooldown starts
    runLine(runner, "wait 500");      // jump cooldown (500ms) fully elapses
    runLine(runner, "click 50 150");
    runLine(runner, "click 150 150");
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("requestJump is rejected while a piece is still cooling down from a previous jump") {
    GameEngine game = makeGame("Board:\nwK . .\n");
    game.requestJump(Position{0, 0}); // airborne until t=1000
    game.wait(1000);                   // lands, jump cooldown (500ms) starts

    game.requestJump(Position{0, 0}); // rejected: still cooling down, must not restart the cooldown window
    game.wait(500);                    // exactly the original jump cooldown's duration

    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 1});
    CHECK(result.is_accepted); // the rejected second jump did not extend the cooldown
}

TEST_CASE("requestMove rejects with cooling_down for a piece still resting after its last move") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    game.requestMove(Position{0, 0}, Position{0, 1}); // 1000ms
    game.wait(1000);                                   // arrives, cooldown starts

    MoveResult result = game.requestMove(Position{0, 1}, Position{0, 2});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveResultReason::CoolingDown);
}

TEST_CASE("redirect is blocked even when the new destination would otherwise be a legal move") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move to (0,2), 2000ms
    runLine(runner, "click 50 50");   // re-select wR (still shown at origin)
    runLine(runner, "click 150 50");  // attempt redirect to (0,1): otherwise legal, still blocked
    runLine(runner, "wait 2000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("a piece of the opposite color can move concurrently while another piece is mid-transit") {
    GameEngine game = makeGame("Board:\nwR . .\n. . .\nbR . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move wR to (0,2), 2000ms
    runLine(runner, "click 50 250");  // select bR at (2,0)
    runLine(runner, "click 250 250"); // bR to (2,2), 2000ms — a different piece, runs alongside wR
    runLine(runner, "wait 2000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\n. . .\n. . bR\n");
}

TEST_CASE("a piece of the same color can also move concurrently while another piece is mid-transit") {
    GameEngine game = makeGame("Board:\nwR . .\n. . .\nwN . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move wR to (0,2), 2000ms
    runLine(runner, "click 50 250");  // select wN at (2,0)
    runLine(runner, "click 250 150"); // wN to (1,2), Chebyshev distance 2: 2000ms — runs alongside wR
    runLine(runner, "wait 2000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\n. . wN\n. . .\n");
}

TEST_CASE("capturing the enemy king ends the game") {
    GameEngine game = makeGame("Board:\nwR . bK\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move wR to (0,2): captures bK, 2000ms
    runLine(runner, "wait 2000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("after the game is over, further click and wait commands are ignored") {
    GameEngine game = makeGame("Board:\nwR . bK\nbR . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move wR to (0,2): captures bK, 2000ms
    runLine(runner, "wait 2000");     // game ends here: white wins
    runLine(runner, "click 50 150");  // select bR at (1,0): must be ignored
    runLine(runner, "click 150 150"); // attempt to move bR to (1,1): must be ignored
    runLine(runner, "wait 1000");     // must not resolve any move
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\nbR . .\n");
}

TEST_CASE("black capturing white's king reports black as the winner") {
    GameEngine game = makeGame("Board:\nbR . wK\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select bR at (0,0)
    runLine(runner, "click 250 50");  // move bR to (0,2): captures wK, 2000ms
    runLine(runner, "wait 2000");
    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::Black);
}

TEST_CASE("a pawn can move two cells from its start row and takes proportionally longer to arrive") {
    // 5 rows: the start row is the bottom edge (row 4), and the destination
    // row 2 is not the promotion row, so this test isolates movement/timing
    // from the separate promotion behavior.
    GameEngine game = makeGame("Board:\n. . .\n. . .\n. . .\n. . .\nwP . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 450"); // select wP at (4,0), its start row
    runLine(runner, "click 50 250"); // move to (2,0): Chebyshev distance 2, 2000ms
    runLine(runner, "wait 1000");    // half the duration: shown at the intermediate cell
    runLine(runner, "print board");
    runLine(runner, "wait 1000");    // now the move fully arrives
    runLine(runner, "print board");
    CHECK(out.str() ==
        ". . .\n. . .\n. . .\nwP . .\n. . .\n"
        ". . .\n. . .\nwP . .\n. . .\n. . .\n");
}

TEST_CASE("a pawn cannot move two cells from a row that is not its start row") {
    GameEngine game = makeGame("Board:\n. . .\n. . .\nwP . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 250"); // select wP at (2,0): start row is (3), not (2)
    runLine(runner, "click 50 50");  // attempt move to (0,0): illegal, no pending move
    runLine(runner, "wait 5000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\n. . .\nwP . .\n. . .\n");
}

TEST_CASE("a white pawn becomes a queen when it reaches the last row") {
    GameEngine game = makeGame("Board:\n. . .\nwP . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 150"); // select wP at (1,0)
    runLine(runner, "click 50 50");  // move to (0,0): its promotion row, 1000ms
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == "wQ . .\n. . .\n");
}

TEST_CASE("a black pawn becomes a queen when it reaches the last row") {
    GameEngine game = makeGame("Board:\nbP . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select bP at (0,0)
    runLine(runner, "click 50 150");  // move to (1,0): its promotion row, 1000ms
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\nbQ . .\n");
}

TEST_CASE("a pawn that captures diagonally into the last row also becomes a queen") {
    GameEngine game = makeGame("Board:\nbR . .\n. wP .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 150 150"); // select wP at (1,1)
    runLine(runner, "click 50 50");   // capture bR at (0,0): its promotion row, 1000ms
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == "wQ . .\n. . .\n");
}

TEST_CASE("isGameOver and winner report correctly before and after white wins") {
    GameEngine game = makeGame("Board:\nwR . bK\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    CHECK_FALSE(game.isGameOver());
    CHECK_FALSE(game.winner().has_value());

    runLine(runner, "click 50 50");
    runLine(runner, "click 250 50");
    runLine(runner, "wait 2000");

    CHECK(game.isGameOver());
    REQUIRE(game.winner().has_value());
    CHECK(*game.winner() == PieceColor::White);
}

TEST_CASE("a jump with nothing arriving lands the piece back on its own square") {
    GameEngine game = makeGame("Board:\n. . .\n. wK .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 150 150"); // wK at (1,1) jumps in place
    runLine(runner, "wait 1000");    // jump duration elapses, no arrival
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("an airborne piece captures an enemy that arrives at its cell before it lands") {
    GameEngine game = makeGame("Board:\n. . .\nwK bR .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 50 150");   // wK at (1,0) jumps, airborne until t=1000
    runLine(runner, "click 150 150"); // select bR at (1,1)
    runLine(runner, "click 50 150");  // bR moves onto wK's cell: distance 1, arrives at t=1000
    runLine(runner, "wait 1000");     // move and landing coincide: defense wins the tie
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\nwK . .\n. . .\n");
}

TEST_CASE("jumping after a capture has already resolved does not undo it") {
    GameEngine game = makeGame("Board:\n. . .\nwK bR .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 150 150"); // select bR at (1,1)
    runLine(runner, "click 50 150");  // bR moves onto wK's cell: distance 1, 1000ms
    runLine(runner, "wait 1000");     // wK is captured before any jump is attempted
    runLine(runner, "jump 50 150");   // too late: wK is already gone
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\nbR . .\n. . .\n");
}

TEST_CASE("an enemy that arrives after a jump has already landed captures normally") {
    GameEngine game = makeGame("Board:\n. . . .\nwK . . bR\n. . . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 50 150");   // wK at (1,0) jumps, airborne until t=1000
    runLine(runner, "wait 1000");     // jump lands normally, wK unprotected again
    runLine(runner, "click 350 150"); // select bR at (1,3)
    runLine(runner, "click 50 150");  // bR moves onto wK's cell: distance 3, 3000ms
    runLine(runner, "wait 3000");     // arrives at t=4000, long after wK landed
    runLine(runner, "print board");
    CHECK(out.str() == ". . . .\nbR . . .\n. . . .\n");
}

TEST_CASE("a piece that is mid-transit cannot jump") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "click 50 50");   // select wR at (0,0)
    runLine(runner, "click 250 50");  // move wR to (0,2): distance 2, 2000ms
    runLine(runner, "wait 500");      // wR is still mid-transit
    runLine(runner, "jump 50 50");    // blocked: a moving piece cannot jump
    runLine(runner, "wait 1500");     // move arrives normally
    runLine(runner, "print board");
    CHECK(out.str() == ". . wR\n");
}

TEST_CASE("an airborne piece only captures an arriving enemy, not a same-color piece that could never legally move there") {
    GameEngine game = makeGame("Board:\n. . .\nwK wR .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 50 150");   // wK at (1,0) jumps
    runLine(runner, "click 150 150"); // select wR at (1,1), same color as wK
    runLine(runner, "click 50 150");  // attempted move onto wK's cell is an illegal same-color capture
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\nwK wR .\n. . .\n");
}

TEST_CASE("jumping on an empty cell does nothing") {
    GameEngine game = makeGame("Board:\n. . .\nwK . .\n. . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 250 150"); // (1,2) is empty
    runLine(runner, "wait 1000");
    runLine(runner, "print board");
    CHECK(out.str() == ". . .\nwK . .\n. . .\n");
}

TEST_CASE("jumping at an out-of-bounds pixel is ignored") {
    GameEngine game = makeGame("Board:\nwK . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump -10 50");
    runLine(runner, "print board");
    CHECK(out.str() == "wK . .\n");
}

TEST_CASE("jumping an already-airborne piece is ignored and does not extend its protection") {
    GameEngine game = makeGame("Board:\n. . . .\nwK . . bR\n. . . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    runLine(runner, "jump 50 150");   // wK at (1,0) jumps, airborne until t=1000
    runLine(runner, "jump 50 150");   // duplicate attempt: ignored, does not reset the timer
    runLine(runner, "wait 1000");     // original jump lands normally, wK unprotected again
    runLine(runner, "click 350 150"); // select bR at (1,3)
    runLine(runner, "click 50 150");  // bR moves onto wK's cell: distance 3, 3000ms
    runLine(runner, "wait 3000");     // arrives at t=4000, well after wK's original window
    runLine(runner, "print board");
    CHECK(out.str() == ". . . .\nbR . . .\n. . . .\n");
}

TEST_CASE("a king intercepted by an enemy's active jump loses the game immediately") {
    GameEngine game = makeGame("Board:\nwK bP .\n");
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
// terms, with no pixels and no ScriptRunner/Controller involved.

TEST_CASE("requestMove accepts a legal move and returns ok") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 2});
    CHECK(result.is_accepted);
    CHECK(result.reason == MoveResultReason::Ok);
}

TEST_CASE("requestMove rejects an illegal move and returns the rule-level reason") {
    GameEngine game = makeGame("Board:\nwR . wP\n");
    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 2});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveRejectionReason::FriendlyDestination);
}

TEST_CASE("requestMove rejects with game_over once the game has ended") {
    GameEngine game = makeGame("Board:\nwR . bK\n");
    game.requestMove(Position{0, 0}, Position{0, 2}); // captures bK, 2000ms
    game.wait(2000);
    REQUIRE(game.isGameOver());

    MoveResult result = game.requestMove(Position{0, 2}, Position{0, 1});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveResultReason::GameOver);
}

TEST_CASE("requestMove accepts a different piece while another move is in flight") {
    GameEngine game = makeGame("Board:\nwR . .\n. . .\nbR . .\n");
    game.requestMove(Position{0, 0}, Position{0, 2}); // wR: 2000ms in flight

    MoveResult result = game.requestMove(Position{2, 0}, Position{2, 2});
    CHECK(result.is_accepted);
    CHECK(result.reason == MoveResultReason::Ok);
}

TEST_CASE("requestMove rejects with motion_in_progress for the same piece that is already in flight") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    game.requestMove(Position{0, 0}, Position{0, 2}); // wR: 2000ms in flight

    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 1});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveResultReason::MotionInProgress);
}

TEST_CASE("two same-color paths crossing mid-flight: the later one stops short, end-to-end through requestMove/wait") {
    GameEngine game = makeGame("Board:\n. . wR . .\n. . . . .\n. . . . wR\n. . . . .\n. . . . .\n");

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
    GameEngine game = makeGame("Board:\nwR .\n. .\n. wR\n");

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
    GameEngine game = makeGame("Board:\nwK .\n. .\n. .\n. bR\n");

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
    GameEngine game = makeGame("Board:\n. . .\nwK bR .\n. . .\n");
    game.requestJump(Position{1, 0}); // wK jumps, airborne until t=1000

    // bR moving onto wK's cell is intercepted while the jump is active.
    game.requestMove(Position{1, 1}, Position{1, 0});
    game.wait(1000);

    CHECK(game.hasPieceAt(Position{1, 0}));
    CHECK_FALSE(game.hasPieceAt(Position{1, 1}));
}

TEST_CASE("a printed board still shows a mid-flight piece at its source until it arrives") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    game.requestMove(Position{0, 0}, Position{0, 2}); // 2000ms in flight
    runLine(runner, "wait 500");              // mid-flight
    runLine(runner, "print board");
    CHECK(out.str() == "wR . .\n");
}

TEST_CASE("a mid-flight piece's source cell reads as empty to hasPieceAt") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    game.requestMove(Position{0, 0}, Position{0, 2}); // 2000ms in flight
    game.wait(500);

    // The piece has left the board for the duration of its flight — it can
    // no longer be selected or commanded, which is exactly the movement
    // lock: a piece mid-walk cannot be told to go somewhere new.
    CHECK_FALSE(game.hasPieceAt(Position{0, 0}));
}

TEST_CASE("requestMove on a mid-jump piece is rejected with motion_in_progress") {
    GameEngine game = makeGame("Board:\nwK . .\n");
    game.requestJump(Position{0, 0}); // airborne until t=1000

    MoveResult result = game.requestMove(Position{0, 0}, Position{0, 1});
    CHECK_FALSE(result.is_accepted);
    CHECK(result.reason == MoveResultReason::MotionInProgress);
}

TEST_CASE("a king mid-jump never disappears from the printed board") {
    GameEngine game = makeGame("Board:\nwK . bR\n");
    Controller controller = makeController(game);
    std::ostringstream out;
    ScriptRunner runner(controller, game, out);
    game.requestJump(Position{0, 0}); // airborne until t=1000
    runLine(runner, "wait 500");
    runLine(runner, "print board");
    CHECK(out.str() == "wK . bR\n");
}

TEST_CASE("hasPieceAt distinguishes occupied cells from empty ones") {
    GameEngine game = makeGame("Board:\nwK . .\n");
    CHECK(game.hasPieceAt(Position{0, 0}));
    CHECK_FALSE(game.hasPieceAt(Position{0, 1}));
}

TEST_CASE("snapshot reflects the current board and game-over state") {
    GameEngine game = makeGame("Board:\nwR . bK\n");

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

TEST_CASE("a piece is LongRest after a move lands and returns to Idle once its cooldown ends") {
    GameEngine game = makeGame("Board:\nwR . .\n. . .\n. . .\n");
    game.requestMove(Position{0, 0}, Position{0, 1});
    game.wait(1000); // the one-cell move lands

    GameSnapshot afterLanding = game.snapshot();
    const Piece* moved = afterLanding.board().pieceAt(0, 1);
    REQUIRE(moved != nullptr);
    CHECK(moved->state() == Piece::State::LongRest);

    game.wait(1000); // the move cooldown fully elapses
    GameSnapshot afterCooldown = game.snapshot();
    const Piece* rested = afterCooldown.board().pieceAt(0, 1);
    REQUIRE(rested != nullptr);
    CHECK(rested->state() == Piece::State::Idle);
}

TEST_CASE("a piece is ShortRest after a jump lands and returns to Idle after the shorter jump cooldown") {
    GameEngine game = makeGame("Board:\nwR . .\n. . .\n. . .\n");
    game.requestJump(Position{0, 0});
    game.wait(1000); // the jump lands in place

    GameSnapshot afterLanding = game.snapshot();
    const Piece* jumped = afterLanding.board().pieceAt(0, 0);
    REQUIRE(jumped != nullptr);
    CHECK(jumped->state() == Piece::State::ShortRest);

    game.wait(500); // the jump cooldown (shorter than a move's) elapses
    GameSnapshot afterCooldown = game.snapshot();
    const Piece* rested = afterCooldown.board().pieceAt(0, 0);
    REQUIRE(rested != nullptr);
    CHECK(rested->state() == Piece::State::Idle);
}

TEST_CASE("a stale cooldown expiring does not wake a different piece captured onto its cell") {
    GameEngine game = makeGame("Board:\nwR . bR\n. . .\n. . .\n");
    game.requestMove(Position{0, 0}, Position{0, 1}); // white settles on the middle cell
    game.wait(1000);                                   // white LongRest, cooldown ends at t=2000

    game.requestMove(Position{0, 2}, Position{0, 1}); // black moves onto white's cell
    game.wait(1000);                                   // at t=2000 black captures white as white's stale cooldown fires

    GameSnapshot snap = game.snapshot();
    const Piece* occupant = snap.board().pieceAt(0, 1);
    REQUIRE(occupant != nullptr);
    CHECK(occupant->color() == PieceColor::Black);        // black captured white
    CHECK(occupant->state() == Piece::State::LongRest);   // its own rest, not reset by white's stale expiry
}
