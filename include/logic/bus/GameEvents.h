#pragma once
#include "logic/model/PieceColor.h"
#include "logic/model/PieceType.h"
#include "logic/model/Position.h"

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

struct GameStartedEvent {
};

struct GameEndedEvent {
    PieceColor winner;
};
