#include <iostream>
#include "logic/io/BoardParser.h"
#include "logic/io/BoardPrinter.h"
#include "logic/engine/GameEngine.h"
#include "client/input/BoardMapper.h"
#include "client/input/Controller.h"
#include "texttests/ScriptRunner.h"

namespace {
    constexpr int kCellSizePixels = 100;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    ParsedInput parsed;
    std::string errorMessage;
    if (!BoardParser::parse(std::cin, parsed, errorMessage)) {
        printParseError(errorMessage, std::cout);
        return 0;
    }

    int rows = parsed.board.rowCount();
    int cols = parsed.board.colCount();
    GameEngine game(std::move(parsed.board));
    Controller controller(game, BoardMapper(rows, cols, kCellSizePixels));
    ScriptRunner runner(controller, game, std::cout);

    for (const auto& line : parsed.commands) {
        runLine(runner, line);
    }

    return 0;
}