#pragma once
#include <optional>
#include <string>
#include "model/Piece.h"
#include "model/PieceColor.h"
#include "model/PieceType.h"

struct PieceCode {
    PieceColor color;
    PieceType kind;
};

bool isEmptyToken(const std::string& token);
std::optional<PieceCode> pieceFromToken(const std::string& token);
std::string encodeCell(const Piece* piece);
