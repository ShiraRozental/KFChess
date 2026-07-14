#include "doctest/doctest.h"
#include "app/Controller.h"
#include "engine/GameEngine.h"

namespace {
    constexpr int kCellSizePixels = 100;
}

TEST_CASE("a first click on an occupied cell selects it") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(50, 50); // (0,0)

    REQUIRE(controller.selectedCell().has_value());
    CHECK(*controller.selectedCell() == Position{0, 0});
}

TEST_CASE("a first click on an empty cell leaves the selection empty") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(250, 250); // (2,2), empty

    CHECK_FALSE(controller.selectedCell().has_value());
}

TEST_CASE("a click outside the board with no selection does nothing") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(-10, 50);

    CHECK_FALSE(controller.selectedCell().has_value());
}

TEST_CASE("a click outside the board with a selection clears it without requesting a move") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.click(50, 50);   // select wK
    controller.click(-10, 50);  // outside board: cancels

    CHECK_FALSE(controller.selectedCell().has_value());

    game.wait(5000);
    CHECK(game.hasPieceAt(Position{0, 0})); // wK never moved
}

TEST_CASE("a second click inside the board on a legal destination requests the move and applies it") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n. . .\n. . .\n", error);
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
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR bP .\n", error);
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
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwR . wK\n. . .\n", error);
    Controller controller(game, BoardMapper(2, 3, kCellSizePixels));

    controller.click(50, 50);                             // select wR at (0,0)
    ControllerResult result = controller.click(250, 50);  // wK, same color

    CHECK(result.requestedMove);
    CHECK_FALSE(result.moveResult.is_accepted);
    CHECK_FALSE(controller.selectedCell().has_value());
}

TEST_CASE("jump translates a pixel into a cell and requests a jump, independent of selection") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\n. . .\nwK . .\n. . .\n", error);
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));

    controller.jump(50, 150); // wK at (1,0)

    game.wait(1000);
    CHECK(game.hasPieceAt(Position{1, 0})); // lands back on its own square
}

TEST_CASE("jump at an out-of-bounds pixel is ignored") {
    GameEngine game;
    std::string error;
    game.loadBoard("Board:\nwK . .\n", error);
    Controller controller(game, BoardMapper(1, 3, kCellSizePixels));

    controller.jump(-10, 50);

    game.wait(1000);
    CHECK(game.hasPieceAt(Position{0, 0})); // unaffected
}
