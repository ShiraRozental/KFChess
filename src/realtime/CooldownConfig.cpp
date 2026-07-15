#include "realtime/CooldownConfig.h"

namespace {
    constexpr long long kDefaultCooldownMs = 1000;
    constexpr long long kJumpCooldownMs = 500;
}

long long cooldownDurationMsFor(PieceType type) {
    switch (type) {
        case PieceType::King:   return kDefaultCooldownMs;
        case PieceType::Queen:  return kDefaultCooldownMs;
        case PieceType::Rook:   return kDefaultCooldownMs;
        case PieceType::Bishop: return kDefaultCooldownMs;
        case PieceType::Knight: return kDefaultCooldownMs;
        case PieceType::Pawn:   return kDefaultCooldownMs;
    }
    return kDefaultCooldownMs;
}

long long jumpCooldownDurationMs() {
    return kJumpCooldownMs;
}
