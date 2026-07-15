#include "doctest/doctest.h"
#include "io/PieceNotation.h"

TEST_CASE("an empty token is recognized as empty") {
    CHECK(isEmptyToken("."));
}

TEST_CASE("a piece token is not an empty token") {
    CHECK_FALSE(isEmptyToken("wR"));
}

TEST_CASE("a white piece token decodes to the correct color and kind") {
    std::optional<PieceCode> code = pieceFromToken("wR");
    REQUIRE(code.has_value());
    CHECK(code->color == PieceColor::White);
    CHECK(code->kind == PieceType::Rook);
}

TEST_CASE("a black piece token decodes to the correct color and kind") {
    std::optional<PieceCode> code = pieceFromToken("bK");
    REQUIRE(code.has_value());
    CHECK(code->color == PieceColor::Black);
    CHECK(code->kind == PieceType::King);
}

TEST_CASE("an unknown piece letter is rejected") {
    CHECK_FALSE(pieceFromToken("wZ").has_value());
}

TEST_CASE("an unknown color letter is rejected") {
    CHECK_FALSE(pieceFromToken("xR").has_value());
}

TEST_CASE("a token of the wrong length is rejected") {
    CHECK_FALSE(pieceFromToken("w").has_value());
    CHECK_FALSE(pieceFromToken("wRR").has_value());
}

TEST_CASE("encodeCell renders an empty cell as a dot") {
    CHECK(encodeCell(nullptr) == ".");
}

TEST_CASE("encodeCell renders an occupied cell as color plus kind") {
    Piece piece(0, PieceColor::White, PieceType::Queen, Position{0, 0});
    CHECK(encodeCell(&piece) == "wQ");
}
