#include "doctest/doctest.h"
#include "PieceMovementRules.h"

TEST_CASE("sliding pieces report that they require a clear path") {
    CHECK(RookMovementRule().requiresClearPath());
    CHECK(BishopMovementRule().requiresClearPath());
    CHECK(QueenMovementRule().requiresClearPath());
}

TEST_CASE("non-sliding pieces do not require a clear path") {
    CHECK_FALSE(KingMovementRule().requiresClearPath());
    CHECK_FALSE(KnightMovementRule().requiresClearPath());
}
