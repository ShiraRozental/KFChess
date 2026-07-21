#include "logic/realtime/CooldownConfig.h"

namespace {
    constexpr long long kJumpCooldownMs = 500;
}

long long jumpCooldownDurationMs() {
    return kJumpCooldownMs;
}
