#include "doctest/doctest.h"
#include "engine/GameEngine.h"
#include "input/MouseInputRouter.h"
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

    Piece::State stateAt(const GameEngine& game, const Position& cell) {
        const Piece* piece = game.snapshot().board().pieceAt(cell.row, cell.col);
        REQUIRE(piece != nullptr);
        return piece->state();
    }
}

TEST_CASE("two left clicks route through the select/move flow and apply the move") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));
    MouseInputRouter router(controller);

    router.handle(MouseButton::Left, 50, 50);   // select wK at (0,0)
    router.handle(MouseButton::Left, 150, 150); // move to (1,1)

    game.wait(1000);
    CHECK(game.hasPieceAt(Position{1, 1}));
    CHECK_FALSE(game.hasPieceAt(Position{0, 0}));
}

TEST_CASE("a left click selects and a left click outside the board clears the selection") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));
    MouseInputRouter router(controller);

    router.handle(MouseButton::Left, 50, 50);
    REQUIRE(controller.selectedCell().has_value());
    CHECK(*controller.selectedCell() == Position{0, 0});

    router.handle(MouseButton::Left, -10, 50);
    CHECK_FALSE(controller.selectedCell().has_value());
}

TEST_CASE("a right click routes to jump and leaves the selection untouched") {
    GameEngine game = makeGame("Board:\nwK . wR\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));
    MouseInputRouter router(controller);

    router.handle(MouseButton::Left, 50, 50);   // select wK at (0,0)
    router.handle(MouseButton::Right, 250, 50); // jump wR at (0,2)

    CHECK(stateAt(game, Position{0, 2}) == Piece::State::Jumping);
    REQUIRE(controller.selectedCell().has_value());
    CHECK(*controller.selectedCell() == Position{0, 0});
}

TEST_CASE("a right click on an empty cell changes nothing") {
    GameEngine game = makeGame("Board:\nwK . .\n. . .\n. . .\n");
    Controller controller(game, BoardMapper(3, 3, kCellSizePixels));
    MouseInputRouter router(controller);

    router.handle(MouseButton::Right, 150, 150); // (1,1), empty

    CHECK(stateAt(game, Position{0, 0}) == Piece::State::Idle);
    CHECK_FALSE(controller.selectedCell().has_value());
}
