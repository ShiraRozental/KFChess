#include "logic/model/PieceValues.h"

namespace {
    constexpr int kPawnCost = 1;
    constexpr int kKnightCost = 3;
    constexpr int kBishopCost = 3;
    constexpr int kRookCost = 5;
    constexpr int kQueenCost = 9;
    constexpr int kKingCost = 0;
}

int pieceCostOf(PieceType type) {
    switch (type) {
        case PieceType::Pawn:   return kPawnCost;
        case PieceType::Knight: return kKnightCost;
        case PieceType::Bishop: return kBishopCost;
        case PieceType::Rook:   return kRookCost;
        case PieceType::Queen:  return kQueenCost;
        case PieceType::King:   return kKingCost;
    }
    return 0;
}
