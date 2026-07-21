#pragma once
#include "logic/model/PieceColor.h"
#include "logic/model/PieceType.h"
#include "logic/rules/MovementRule.h"
#include <memory>

struct PieceDefinition {
    PieceType type;
    char      symbol;
    int       captureCost;
    long long cooldownMs;
    PieceType promotesTo;
    std::shared_ptr<const MovementRule> whiteMovement;
    std::shared_ptr<const MovementRule> blackMovement;

    const MovementRule& movementFor(PieceColor color) const {
        return color == PieceColor::White ? *whiteMovement : *blackMovement;
    }

    bool promotes() const { return promotesTo != type; }
};
