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

TEST_CASE("pawns do not require a clear path") {
    CHECK_FALSE(WhitePawnMovementRule().requiresClearPath());
    CHECK_FALSE(BlackPawnMovementRule().requiresClearPath());
}

TEST_CASE("white pawn moves one cell toward decreasing row") {
    WhitePawnMovementRule rule;
    CHECK(rule.isLegalMove(3, 3, 2, 3));
}

TEST_CASE("white pawn cannot move two cells") {
    WhitePawnMovementRule rule;
    CHECK_FALSE(rule.isLegalMove(3, 3, 1, 3));
}

TEST_CASE("white pawn cannot move diagonally without capturing") {
    WhitePawnMovementRule rule;
    CHECK_FALSE(rule.isLegalMove(3, 3, 2, 4));
}

TEST_CASE("white pawn captures one cell diagonally toward decreasing row") {
    WhitePawnMovementRule rule;
    CHECK(rule.isLegalCapture(3, 3, 2, 4));
    CHECK(rule.isLegalCapture(3, 3, 2, 2));
}

TEST_CASE("white pawn cannot capture straight ahead") {
    WhitePawnMovementRule rule;
    CHECK_FALSE(rule.isLegalCapture(3, 3, 2, 3));
}

TEST_CASE("black pawn moves one cell toward increasing row") {
    BlackPawnMovementRule rule;
    CHECK(rule.isLegalMove(3, 3, 4, 3));
}

TEST_CASE("black pawn cannot move two cells") {
    BlackPawnMovementRule rule;
    CHECK_FALSE(rule.isLegalMove(3, 3, 5, 3));
}

TEST_CASE("black pawn cannot move diagonally without capturing") {
    BlackPawnMovementRule rule;
    CHECK_FALSE(rule.isLegalMove(3, 3, 4, 4));
}

TEST_CASE("black pawn captures one cell diagonally toward increasing row") {
    BlackPawnMovementRule rule;
    CHECK(rule.isLegalCapture(3, 3, 4, 4));
    CHECK(rule.isLegalCapture(3, 3, 4, 2));
}

TEST_CASE("black pawn cannot capture straight ahead") {
    BlackPawnMovementRule rule;
    CHECK_FALSE(rule.isLegalCapture(3, 3, 4, 3));
}
