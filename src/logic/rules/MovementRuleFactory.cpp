#include "logic/rules/MovementRuleFactory.h"
#include "logic/rules/PieceRules.h"

const MovementRule& movementRuleFor(PieceType type, PieceColor color) {
    static const KingMovementRule king;
    static const QueenMovementRule queen;
    static const RookMovementRule rook;
    static const BishopMovementRule bishop;
    static const KnightMovementRule knight;
    static const WhitePawnMovementRule whitePawn;
    static const BlackPawnMovementRule blackPawn;

    switch (type) {
        case PieceType::King:   return king;
        case PieceType::Queen:  return queen;
        case PieceType::Rook:   return rook;
        case PieceType::Bishop: return bishop;
        case PieceType::Knight: return knight;
        case PieceType::Pawn:
            if (color == PieceColor::White) return whitePawn;
            return blackPawn;
    }
    return king; // unreachable for valid enum values; keeps all paths returning
}
