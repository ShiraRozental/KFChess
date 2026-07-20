#include "doctest/doctest.h"
#include "input/Controller.h"
#include "engine/GameEngine.h"
#include "io/BoardParser.h"
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
}

TEST_CASE("a first click on an occupied cell selects it") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(50, 50); // (0,0)

    REQUIRE(controller.selectedCell().has_value());
    CHECK(*controller.selectedCell() == Position{0, 0});
}

TEST_CASE("a first click on an empty cell leaves the selection empty") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(250, 250); // (2,2), empty

    CHECK_FALSE(controller.selectedCell().has_value());
}

TEST_CASE("a click outside the board with no selection does nothing") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(-10, 50);

    CHECK_FALSE(controller.selectedCell().has_value());
}

TEST_CASE("a click outside the board with a selection clears it without requesting a move") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(50, 50);   // select wK
    controller.click(-10, 50);  // outside board: cancels

    CHECK_FALSE(controller.selectedCell().has_value());

    game.wait(5000);
    CHECK(game.hasPieceAt(Position{0, 0})); // wK never moved
}

TEST_CASE("a second click inside the board on a legal destination requests the move and applies it") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(50, 50);                              // select wK at (0,0)
    ControllerResult result = controller.click(150, 150);  // move to (1,1)

    CHECK(result.requestedMove);
    CHECK(result.moveResult.is_accepted);
    CHECK_FALSE(controller.selectedCell().has_value());

    game.wait(1000);
    CHECK(game.hasPieceAt(Position{1, 1}));
    CHECK_FALSE(game.hasPieceAt(Position{0, 0}));
}

TEST_CASE("a second click inside the board on an illegal destination still clears the selection") {
    GameEngine game = makeGame("Board:\nwR bP .\n");
    Controller controller(game, BoardMapper(1, 3, kCellSizePixels));

    controller.click(50, 50);                             // select wR at (0,0)
    ControllerResult result = controller.click(250, 50);  // blocked destination

    CHECK(result.requestedMove);
    CHECK_FALSE(result.moveResult.is_accepted);
    CHECK_FALSE(controller.selectedCell().has_value());

    game.wait(5000);
    CHECK(game.hasPieceAt(Position{0, 0})); // wR never moved
}

TEST_CASE("a second click on a same-color piece is rejected, not reselected") {
    GameEngine game = makeGame("Board:\nwR . wK\n. . .\n");
    Controller controller(game, BoardMapper(2, 3, kCellSizePixels));

    controller.click(50, 50);                             // select wR at (0,0)
    ControllerResult result = controller.click(250, 50);  // wK, same color

    CHECK(result.requestedMove);
    CHECK_FALSE(result.moveResult.is_accepted);
    CHECK_FALSE(controller.selectedCell().has_value());
}

TEST_CASE("jump translates a pixel into a cell and requests a jump, independent of selection") {
    GameEngine game = makeGame("Board:\n. . .\nwK . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.jump(50, 150); // wK at (1,0)

    game.wait(1000);
    CHECK(game.hasPieceAt(Position{1, 0})); // lands back on its own square
}

TEST_CASE("jump at an out-of-bounds pixel is ignored") {
    GameEngine game = makeGame("Board:\nwK . .\n");
    Controller controller(game, BoardMapper(1, 3, kCellSizePixels));

    controller.jump(-10, 50);

    game.wait(1000);
    CHECK(game.hasPieceAt(Position{0, 0})); // unaffected
}

TEST_CASE("Controller drives the engine through the IGameEngine interface") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    IGameEngine& engine = game;
    Controller controller(engine, BoardMapper(3, 3, kCellSizePixels));

    controller.click(50, 50);
    ControllerResult result = controller.click(150, 150);

    CHECK(result.moveResult.is_accepted);
    engine.wait(1000);
    CHECK(engine.hasPieceAt(Position{1, 1}));
}
