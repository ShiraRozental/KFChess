#pragma once
#include <optional>
#include <string>
#include <vector>
#include "logic/engine/GameSnapshot.h"
#include "logic/engine/MovesLog.h"

struct GameStateMessage {
    GameSnapshot snapshot;
    std::vector<MoveLogEntry> whiteMoves;
    std::vector<MoveLogEntry> blackMoves;
    int whiteScore;
    int blackScore;
};

std::string encodeGameState(const GameStateMessage& message);
std::optional<GameStateMessage> parseGameState(const std::string& text);
