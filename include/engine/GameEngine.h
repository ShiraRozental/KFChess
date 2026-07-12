#pragma once
#include <string>
#include <ostream>
#include <optional>
#include <vector>
#include "model/Board.h"
#include "model/GameState.h"
#include "model/PieceColor.h"
#include "model/PieceType.h"

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

// A piece that jumped in place and is airborne until endTimeMs. While
// airborne it stays on (row, col): it never moves, but if an enemy's
// pending move arrives at this same cell before it lands, it captures that
// enemy instead of being captured (see GameEngine::applyDueMoves).
struct PendingJump {
    int row;
    int col;
    long long endTimeMs;
};

class GameEngine {
public:
    bool loadBoard(const std::string& boardText, std::string& errorMessage);
    void executeLine(const std::string& line, std::ostream& out);
    bool isGameOver() const;
    std::optional<PieceColor> winner() const;

private:
    void handleClick(int pixelX, int pixelY);
    void handleWait(int ms);
    void handleJump(int pixelX, int pixelY);
    void handlePrintBoard(std::ostream& out);
    void applyDueMoves();
    void applyDueJumps();
    void promoteIfNeeded(int row, int col, PieceType type, PieceColor color);
    bool isAnyMovePending() const;
    bool isPieceMoving(int row, int col) const;
    bool isPieceAirborne(int row, int col) const;
    static GameState winningStateFor(PieceColor color);

    Board board_;
    std::optional<Position> selected_;
    long long clockMs_ = 0;
    std::vector<PendingMove> pendingMoves_;
    std::vector<PendingJump> pendingJumps_;
    GameState gameState_ = GameState::InProgress;
};