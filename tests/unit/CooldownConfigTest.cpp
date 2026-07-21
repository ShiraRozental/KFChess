#include "doctest/doctest.h"
#include "logic/realtime/CooldownConfig.h"
#include "logic/config/PieceCatalog.h"
#include <array>

namespace {
    const std::array<PieceType, 6> kAllTypes = {
        PieceType::King, PieceType::Queen, PieceType::Rook,
        PieceType::Bishop, PieceType::Knight, PieceType::Pawn};
}

TEST_CASE("the jump cooldown is positive") {
    CHECK(jumpCooldownDurationMs() > 0);
}

TEST_CASE("the jump cooldown is shorter than every piece's move cooldown") {
    const PieceCatalog& catalog = PieceCatalog::standard();
    for (PieceType type : kAllTypes) {
        CHECK(jumpCooldownDurationMs() < catalog.definitionFor(type).cooldownMs);
    }
}
