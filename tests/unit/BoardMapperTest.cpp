#include "doctest/doctest.h"
#include "input/BoardMapper.h"

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
