#pragma once
#include <optional>
#include <string>
#include "model/Piece.h"
#include "model/PieceColor.h"
#include "model/PieceType.h"
#include "model/Position.h"

struct PieceCode {
    PieceColor color;
    PieceType kind;
};

bool isEmptyToken(const std::string& token);
std::optional<PieceCode> pieceFromToken(const std::string& token);
std::string encodeCell(const Piece* piece);

char pieceTypeToChar(PieceType type);
char pieceColorToChar(PieceColor color);

std::string algebraicCell(const Position& cell, int boardRowCount);
std::string moveText(PieceType kind, const Position& destination, bool isJump,
                     int boardRowCount);
