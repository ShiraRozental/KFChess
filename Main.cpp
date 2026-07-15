#include <iostream>
#include "io/BoardParser.h"
#include "engine/GameEngine.h"
#include "texttests/TextTestRunner.h"
//githube:
//https://github.com/ShiraRozental/KFChess
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string boardText;
    std::vector<std::string> commandLines;
    BoardParser::parse(std::cin, boardText, commandLines);

    GameEngine game;
    std::string errorMessage;
    if (!game.loadBoard(boardText, errorMessage)) {
        std::cout << errorMessage << "\n";
        return 0;
    }

    TextTestRunner runner(game);
    for (const auto& line : commandLines) {
        runner.executeLine(line, std::cout);
    }

    return 0;
}