#include "doctest/doctest.h"
#include "logic/io/PieceNotation.h"

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

TEST_CASE("fileLabel maps a column to its letter") {
    CHECK(fileLabel(0) == "a");
    CHECK(fileLabel(7) == "h");
}

TEST_CASE("rankLabel counts ranks from the bottom") {
    CHECK(rankLabel(0, 8) == "8");
    CHECK(rankLabel(7, 8) == "1");
}

TEST_CASE("algebraicCell maps the board corners with an inverted rank") {
    CHECK(algebraicCell(Position{7, 0}, 8) == "a1");
    CHECK(algebraicCell(Position{0, 7}, 8) == "h8");
    CHECK(algebraicCell(Position{0, 0}, 8) == "a8");
    CHECK(algebraicCell(Position{7, 7}, 8) == "h1");
}

TEST_CASE("algebraicCell maps a middle cell") {
    CHECK(algebraicCell(Position{3, 4}, 8) == "e5");
}

TEST_CASE("algebraicCell follows the given board height") {
    CHECK(algebraicCell(Position{0, 0}, 4) == "a4");
}

TEST_CASE("moveText renders a pawn move as the bare destination") {
    CHECK(moveText(PieceType::Pawn, Position{3, 4}, false, 8) == "e5");
}

TEST_CASE("moveText prefixes a non-pawn move with the piece letter") {
    CHECK(moveText(PieceType::Knight, Position{5, 5}, false, 8) == "Nf3");
}

TEST_CASE("moveText renders a jump as Jump regardless of destination") {
    CHECK(moveText(PieceType::Queen, Position{3, 4}, true, 8) == "Jump");
}

TEST_CASE("parseAlgebraicCell is the inverse of algebraicCell") {
    std::optional<Position> cell = parseAlgebraicCell("e5", 8);
    REQUIRE(cell.has_value());
    CHECK(*cell == Position{3, 4});
    CHECK(algebraicCell(*cell, 8) == "e5");
}

TEST_CASE("parseAlgebraicCell maps the board corners") {
    CHECK(parseAlgebraicCell("a1", 8) == std::optional<Position>{Position{7, 0}});
    CHECK(parseAlgebraicCell("h8", 8) == std::optional<Position>{Position{0, 7}});
}

TEST_CASE("parseAlgebraicCell rejects malformed or off-board cells") {
    CHECK_FALSE(parseAlgebraicCell("", 8).has_value());
    CHECK_FALSE(parseAlgebraicCell("2", 8).has_value());     // no file letter
    CHECK_FALSE(parseAlgebraicCell("e", 8).has_value());     // no rank digit
    CHECK_FALSE(parseAlgebraicCell("e0", 8).has_value());    // rank below range
    CHECK_FALSE(parseAlgebraicCell("e9", 8).has_value());    // rank above range
    CHECK_FALSE(parseAlgebraicCell("i2", 8).has_value());    // file beyond board
}

TEST_CASE("pieceTypeFromChar is the inverse of pieceTypeToChar") {
    CHECK(pieceTypeFromChar('Q') == std::optional<PieceType>{PieceType::Queen});
    CHECK(pieceTypeFromChar('N') == std::optional<PieceType>{PieceType::Knight});
    CHECK_FALSE(pieceTypeFromChar('Z').has_value());
}
