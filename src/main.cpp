#include <iostream>
#include "InputParser.h"
#include "Game.h"

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string boardText;
    std::vector<std::string> commandLines;
    InputParser::parse(std::cin, boardText, commandLines);

    Game game;
    std::string errorMessage;
    if (!game.loadBoard(boardText, errorMessage)) {
        std::cout << errorMessage << "\n";
        return 0;
    }

    for (const auto& line : commandLines) {
        game.executeLine(line, std::cout);
    }

    return 0;
}