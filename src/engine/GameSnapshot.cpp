#include "engine/GameSnapshot.h"

GameSnapshot::GameSnapshot(Board board, bool gameOver, std::optional<PieceColor> winner)
    : board_(std::move(board)), gameOver_(gameOver), winner_(winner) {
}

const Board& GameSnapshot::board() const { return board_; }
bool GameSnapshot::isGameOver() const { return gameOver_; }
std::optional<PieceColor> GameSnapshot::winner() const { return winner_; }
