#pragma once
#include <optional>
#include <string>
#include "logic/model/PieceColor.h"
#include "logic/model/PieceType.h"
#include "logic/model/Position.h"

enum class WireCommandType { Move, Jump };

struct WireCommand {
    WireCommandType type;
    PieceColor color;
    PieceType piece;
    Position from;
    Position to;
};

bool operator==(const WireCommand& lhs, const WireCommand& rhs);

char wireColorToChar(PieceColor color);
std::optional<PieceColor> wireColorFromChar(char c);

std::string encodeMove(PieceColor color, PieceType piece,
                       const Position& from, const Position& to, int boardRowCount);
std::string encodeJump(PieceColor color, PieceType piece,
                       const Position& cell, int boardRowCount);
std::optional<WireCommand> parseCommand(const std::string& text, int boardRowCount);
