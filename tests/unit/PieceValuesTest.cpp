#include "doctest/doctest.h"
#include "logic/model/PieceValues.h"

TEST_CASE("each piece type has its configured capture cost") {
    CHECK(pieceCostOf(PieceType::Pawn) == 1);
    CHECK(pieceCostOf(PieceType::Knight) == 3);
    CHECK(pieceCostOf(PieceType::Bishop) == 3);
    CHECK(pieceCostOf(PieceType::Rook) == 5);
    CHECK(pieceCostOf(PieceType::Queen) == 9);
}

TEST_CASE("a captured king is worth no points") {
    CHECK(pieceCostOf(PieceType::King) == 0);
}
