#include "doctest/doctest.h"
#include "client/input/BoardMapper.h"

TEST_CASE("x=0..99 maps to column 0") {
    BoardMapper mapper(8, 8, 100);
    CHECK(mapper.cellAt(0, 0)->col == 0);
    CHECK(mapper.cellAt(99, 0)->col == 0);
}

TEST_CASE("x=100..199 maps to column 1") {
    BoardMapper mapper(8, 8, 100);
    CHECK(mapper.cellAt(100, 0)->col == 1);
    CHECK(mapper.cellAt(199, 0)->col == 1);
}

TEST_CASE("y=100..199 maps to row 1") {
    BoardMapper mapper(8, 8, 100);
    CHECK(mapper.cellAt(0, 100)->row == 1);
    CHECK(mapper.cellAt(0, 199)->row == 1);
}

TEST_CASE("a negative pixel is rejected instead of wrapping to cell 0") {
    BoardMapper mapper(8, 8, 100);
    CHECK_FALSE(mapper.cellAt(-10, 0).has_value());
    CHECK_FALSE(mapper.cellAt(0, -10).has_value());
}

TEST_CASE("a pixel exactly at the outer edge of the board is rejected") {
    BoardMapper mapper(3, 3, 100);
    CHECK(mapper.cellAt(299, 0).has_value());
    CHECK_FALSE(mapper.cellAt(300, 0).has_value());
    CHECK(mapper.cellAt(0, 299).has_value());
    CHECK_FALSE(mapper.cellAt(0, 300).has_value());
}

TEST_CASE("a pixel inside bounds maps to the correct row and column together") {
    BoardMapper mapper(5, 5, 100);
    std::optional<Position> cell = mapper.cellAt(150, 250);
    REQUIRE(cell.has_value());
    CHECK(cell->row == 2);
    CHECK(cell->col == 1);
}

TEST_CASE("topLeftPixelOf maps a cell to its top-left pixel") {
    BoardMapper mapper(8, 8, 100);
    CHECK(mapper.topLeftPixelOf(Position{0, 0}) == PixelPoint{0, 0});
    CHECK(mapper.topLeftPixelOf(Position{2, 3}) == PixelPoint{300, 200});
}

TEST_CASE("topLeftPixelOf is the inverse of cellAt for every cell") {
    BoardMapper mapper(4, 6, 50);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 6; ++col) {
            PixelPoint pixel = mapper.topLeftPixelOf(Position{row, col});
            std::optional<Position> back = mapper.cellAt(pixel.x, pixel.y);
            REQUIRE(back.has_value());
            CHECK(*back == Position{row, col});
        }
    }
}

TEST_CASE("board pixel dimensions are cells times cell size") {
    BoardMapper mapper(4, 6, 50);
    CHECK(mapper.boardPixelWidth() == 300);
    CHECK(mapper.boardPixelHeight() == 200);
    CHECK(mapper.cellSizePixels() == 50);
}

TEST_CASE("an origin offset shifts cellAt so the board starts at the origin") {
    BoardMapper mapper(8, 8, 100, PixelPoint{290, 135});
    std::optional<Position> cell = mapper.cellAt(290, 135);
    REQUIRE(cell.has_value());
    CHECK(*cell == Position{0, 0});
    CHECK(mapper.cellAt(290 + 150, 135 + 250) == Position{2, 1});
}

TEST_CASE("a pixel left of or above the origin is outside the board") {
    BoardMapper mapper(8, 8, 100, PixelPoint{290, 135});
    CHECK_FALSE(mapper.cellAt(289, 135).has_value());
    CHECK_FALSE(mapper.cellAt(290, 134).has_value());
    CHECK_FALSE(mapper.cellAt(10, 10).has_value());
}

TEST_CASE("topLeftPixelOf adds the origin offset") {
    BoardMapper mapper(8, 8, 100, PixelPoint{290, 135});
    CHECK(mapper.topLeftPixelOf(Position{0, 0}) == PixelPoint{290, 135});
    CHECK(mapper.topLeftPixelOf(Position{2, 3}) == PixelPoint{590, 335});
    CHECK(mapper.topLeftPixelOf(BoardPoint{0.5, 0.5}) == PixelPoint{340, 185});
}

TEST_CASE("topLeftPixelOf stays the inverse of cellAt under an origin offset") {
    BoardMapper mapper(4, 6, 50, PixelPoint{77, 33});
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 6; ++col) {
            PixelPoint pixel = mapper.topLeftPixelOf(Position{row, col});
            CHECK(mapper.cellAt(pixel.x, pixel.y) == Position{row, col});
        }
    }
}

TEST_CASE("PixelPoint equality compares both coordinates") {
    CHECK(PixelPoint{1, 2} == PixelPoint{1, 2});
    CHECK(PixelPoint{1, 2} != PixelPoint{2, 1});
    CHECK(PixelPoint{1, 2} != PixelPoint{1, 3});
}
