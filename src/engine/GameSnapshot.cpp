#include "engine/GameSnapshot.h"

GameSnapshot::GameSnapshot(Board board, bool gameOver, std::optional<PieceColor> winner,
                           std::map<PieceId, double> cooldownProgress)
    : board_(std::move(board)), gameOver_(gameOver), winner_(winner),
      cooldownProgress_(std::move(cooldownProgress)) {
}

const Board& GameSnapshot::board() const { return board_; }
bool GameSnapshot::isGameOver() const { return gameOver_; }
std::optional<PieceColor> GameSnapshot::winner() const { return winner_; }

std::optional<double> GameSnapshot::cooldownProgressOf(PieceId pieceId) const {
    auto found = cooldownProgress_.find(pieceId);
    if (found == cooldownProgress_.end()) return std::nullopt;
    return found->second;
}
