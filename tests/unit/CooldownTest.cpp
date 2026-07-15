#include "doctest/doctest.h"
#include "realtime/Cooldown.h"

TEST_CASE("a cooldown has not elapsed before its duration is reached") {
    Cooldown cooldown(1, Position{0, 0}, 1000);
    cooldown.advance(999);
    CHECK_FALSE(cooldown.hasElapsed());
}

TEST_CASE("a cooldown has elapsed exactly when its duration is reached") {
    Cooldown cooldown(1, Position{0, 0}, 1000);
    cooldown.advance(1000);
    CHECK(cooldown.hasElapsed());
}

TEST_CASE("a cooldown accumulates elapsed time across multiple advances") {
    Cooldown cooldown(1, Position{0, 0}, 1000);
    cooldown.advance(400);
    cooldown.advance(400);
    CHECK_FALSE(cooldown.hasElapsed());
    cooldown.advance(200);
    CHECK(cooldown.hasElapsed());
}

TEST_CASE("a cooldown exposes the piece id and cell it was created with") {
    Cooldown cooldown(7, Position{2, 3}, 1000);
    CHECK(cooldown.pieceId() == 7);
    CHECK(cooldown.cell() == Position{2, 3});
}
