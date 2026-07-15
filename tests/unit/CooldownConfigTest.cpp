#include "doctest/doctest.h"
#include "realtime/CooldownConfig.h"

TEST_CASE("every piece type has a positive cooldown duration") {
    CHECK(cooldownDurationMsFor(PieceType::King) > 0);
    CHECK(cooldownDurationMsFor(PieceType::Queen) > 0);
    CHECK(cooldownDurationMsFor(PieceType::Rook) > 0);
    CHECK(cooldownDurationMsFor(PieceType::Bishop) > 0);
    CHECK(cooldownDurationMsFor(PieceType::Knight) > 0);
    CHECK(cooldownDurationMsFor(PieceType::Pawn) > 0);
}
