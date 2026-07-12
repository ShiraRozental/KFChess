#pragma once
#include <string>
#include <ostream>
#include <optional>
#include <vector>
#include "Board.h"

struct Position {
    int row;
    int col;
};

// A move that has been accepted (legality already checked at click time) but
// has not yet reached its destination. The board itself is not mutated until
// arrivalTimeMs is reached, so the piece keeps printing at its origin until then.
struct PendingMove {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
    long long arrivalTimeMs;
};

class Game {
public:
    bool loadBoard(const std::string& boardText, std::string& errorMessage);
    void executeLine(const std::string& line, std::ostream& out);

private:
    void handleClick(int pixelX, int pixelY);
    void handleWait(int ms);
    void handlePrintBoard(std::ostream& out);
    void applyDueMoves();

    Board board_;
    std::optional<Position> selected_;
    long long clockMs_ = 0;
    std::vector<PendingMove> pendingMoves_;
};