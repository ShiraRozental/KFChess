#pragma once
#include <string>
#include <ostream>
#include <optional>
#include "Board.h"

struct Position {
    int row;
    int col;
};

class Game {
public:
    bool loadBoard(const std::string& boardText, std::string& errorMessage);
    void executeLine(const std::string& line, std::ostream& out);

private:
    void handleClick(int pixelX, int pixelY);
    void handleWait(int ms);
    void handlePrintBoard(std::ostream& out);

    Board board_;
    std::optional<Position> selected_;
    long long clockMs_ = 0;
};