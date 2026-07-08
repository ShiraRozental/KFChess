#pragma once
#include "MovementRule.h"
#include "PieceType.h"

// Returns the shared, stateless movement rule for a given piece type.
const MovementRule& movementRuleFor(PieceType type);
