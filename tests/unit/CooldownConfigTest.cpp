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

TEST_CASE("the jump cooldown is positive and shorter than every piece's move cooldown") {
    CHECK(jumpCooldownDurationMs() > 0);
    CHECK(jumpCooldownDurationMs() < cooldownDurationMsFor(PieceType::King));
    CHECK(jumpCooldownDurationMs() < cooldownDurationMsFor(PieceType::Queen));
    CHECK(jumpCooldownDurationMs() < cooldownDurationMsFor(PieceType::Rook));
    CHECK(jumpCooldownDurationMs() < cooldownDurationMsFor(PieceType::Bishop));
    CHECK(jumpCooldownDurationMs() < cooldownDurationMsFor(PieceType::Knight));
    CHECK(jumpCooldownDurationMs() < cooldownDurationMsFor(PieceType::Pawn));
}
