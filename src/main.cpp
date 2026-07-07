#include <iostream>
#include "Board.h"

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Board board;
    std::string errorMessage;

    if (!Board::fromStream(std::cin, board, errorMessage)) {
        std::cout << errorMessage << "\n";
        return 0;
    }
    board.print(std::cout);
    return 0;
}