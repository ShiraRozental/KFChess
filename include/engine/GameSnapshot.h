#pragma once
#include <map>
#include <optional>
#include "model/Board.h"
#include "model/PieceColor.h"

// A detached, read-only copy of the game's current state — never a live
// reference into GameEngine's own Board. Safe to hand to a renderer or
// BoardPrinter: mutating the snapshot's Board can never affect the real
// game, and the real game continuing to run can never invalidate a
// snapshot already handed out.
class GameSnapshot {
public:
    GameSnapshot(Board board, bool gameOver, std::optional<PieceColor> winner,
                 std::map<PieceId, double> cooldownProgress = {});

    const Board& board() const;
    bool isGameOver() const;
    std::optional<PieceColor> winner() const;
    std::optional<double> cooldownProgressOf(PieceId pieceId) const;

private:
    Board board_;
    bool gameOver_;
    std::optional<PieceColor> winner_;
    std::map<PieceId, double> cooldownProgress_;
};
