#pragma once
#include "model/PieceColor.h"
#include "model/PieceType.h"
#include "model/Position.h"

struct MoveAppliedEvent {
    PieceColor color;
    PieceType kind;
    Position from;
    Position to;
    bool isJump;
    long long timestampMs;
};

struct PieceCapturedEvent {
    PieceColor capturedColor;
    PieceType capturedKind;
    PieceColor capturedBy;
    long long timestampMs;
};
