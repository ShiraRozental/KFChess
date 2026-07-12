#pragma once
#include "rules/MovementRule.h"
#include "model/PieceType.h"
#include "model/PieceColor.h"

// Returns the shared, stateless movement rule for a given piece type and
// color. Color only affects the result for Pawn (its direction depends on
// which side it belongs to); every other piece type ignores it.
const MovementRule& movementRuleFor(PieceType type, PieceColor color);
