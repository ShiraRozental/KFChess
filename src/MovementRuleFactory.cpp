#include "MovementRuleFactory.h"
#include "PieceMovementRules.h"

namespace {
    // Placeholder for piece types with no movement pattern defined yet
    // (currently just Pawn). Always rejects the move rather than crashing;
    // pawn movement is a future iteration.
    class NullMovementRule : public MovementRule {
    public:
        bool isLegalMove(int, int, int, int) const override { return false; }
    };
}

const MovementRule& movementRuleFor(PieceType type) {
    static const KingMovementRule king;
    static const QueenMovementRule queen;
    static const RookMovementRule rook;
    static const BishopMovementRule bishop;
    static const KnightMovementRule knight;
    static const NullMovementRule none;

    switch (type) {
        case PieceType::King:   return king;
        case PieceType::Queen:  return queen;
        case PieceType::Rook:   return rook;
        case PieceType::Bishop: return bishop;
        case PieceType::Knight: return knight;
        case PieceType::Pawn:   return none;
    }
    return none; // unreachable for valid enum values; keeps all paths returning
}
