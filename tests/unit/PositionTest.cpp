#include "doctest/doctest.h"
#include "logic/model/Position.h"
#include <sstream>

TEST_CASE("a position equals itself") {
    Position p{2, 3};
    CHECK(p == p);
}

TEST_CASE("positions with the same row and column are equal") {
    Position a{2, 3};
    Position b{2, 3};
    CHECK(a == b);
    CHECK_FALSE(a != b);
}

TEST_CASE("positions with a different row are not equal") {
    Position a{2, 3};
    Position b{5, 3};
    CHECK_FALSE(a == b);
    CHECK(a != b);
}

TEST_CASE("positions with a different column are not equal") {
    Position a{2, 3};
    Position b{2, 7};
    CHECK_FALSE(a == b);
    CHECK(a != b);
}

TEST_CASE("positions with both a different row and column are not equal") {
    Position a{2, 3};
    Position b{5, 7};
    CHECK_FALSE(a == b);
    CHECK(a != b);
}

TEST_CASE("a position streams to a readable representation") {
    std::ostringstream out;
    out << Position{2, 3};
    CHECK(out.str() == "(2, 3)");
}
