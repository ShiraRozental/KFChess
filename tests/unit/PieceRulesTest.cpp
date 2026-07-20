#include "doctest/doctest.h"
#include "logic/rules/PieceRules.h"

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

TEST_CASE("white pawn can move two cells from its start row") {
    WhitePawnMovementRule rule;
    CHECK(rule.isLegalDoubleMove(7, 3, 5, 3, 8));
}

TEST_CASE("white pawn cannot move two cells from a non-start row") {
    WhitePawnMovementRule rule;
    CHECK_FALSE(rule.isLegalDoubleMove(6, 3, 4, 3, 8));
}

TEST_CASE("white pawn cannot move two cells diagonally") {
    WhitePawnMovementRule rule;
    CHECK_FALSE(rule.isLegalDoubleMove(7, 3, 5, 4, 8));
}

TEST_CASE("black pawn can move two cells from its start row") {
    BlackPawnMovementRule rule;
    CHECK(rule.isLegalDoubleMove(0, 3, 2, 3, 8));
}

TEST_CASE("black pawn cannot move two cells from a non-start row") {
    BlackPawnMovementRule rule;
    CHECK_FALSE(rule.isLegalDoubleMove(1, 3, 3, 3, 8));
}

TEST_CASE("black pawn cannot move two cells diagonally") {
    BlackPawnMovementRule rule;
    CHECK_FALSE(rule.isLegalDoubleMove(0, 3, 2, 4, 8));
}

TEST_CASE("white pawn reaches its promotion row only at row 0") {
    WhitePawnMovementRule rule;
    CHECK(rule.reachesPromotionRow(0, 8));
    CHECK_FALSE(rule.reachesPromotionRow(1, 8));
}

TEST_CASE("black pawn reaches its promotion row only at the last row") {
    BlackPawnMovementRule rule;
    CHECK(rule.reachesPromotionRow(7, 8));
    CHECK_FALSE(rule.reachesPromotionRow(6, 8));
}

TEST_CASE("non-pawn rules never report a double move or a promotion row") {
    CHECK_FALSE(KingMovementRule().isLegalDoubleMove(6, 3, 4, 3, 8));
    CHECK_FALSE(QueenMovementRule().isLegalDoubleMove(6, 3, 4, 3, 8));
    CHECK_FALSE(RookMovementRule().isLegalDoubleMove(6, 3, 4, 3, 8));
    CHECK_FALSE(BishopMovementRule().isLegalDoubleMove(6, 3, 4, 3, 8));
    CHECK_FALSE(KnightMovementRule().isLegalDoubleMove(6, 3, 4, 3, 8));

    CHECK_FALSE(KingMovementRule().reachesPromotionRow(0, 8));
    CHECK_FALSE(QueenMovementRule().reachesPromotionRow(0, 8));
    CHECK_FALSE(RookMovementRule().reachesPromotionRow(0, 8));
    CHECK_FALSE(BishopMovementRule().reachesPromotionRow(0, 8));
    CHECK_FALSE(KnightMovementRule().reachesPromotionRow(0, 8));
}
