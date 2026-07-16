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

TEST_CASE("a fresh cooldown reports zero fraction elapsed") {
    Cooldown cooldown(1, Position{0, 0}, 1000);
    CHECK(cooldown.fractionElapsed() == doctest::Approx(0.0));
}

TEST_CASE("a cooldown reports its fraction elapsed mid-way") {
    Cooldown cooldown(1, Position{0, 0}, 1000);
    cooldown.advance(250);
    CHECK(cooldown.fractionElapsed() == doctest::Approx(0.25));
}

TEST_CASE("fraction elapsed is clamped to one past the duration") {
    Cooldown cooldown(1, Position{0, 0}, 1000);
    cooldown.advance(5000);
    CHECK(cooldown.fractionElapsed() == doctest::Approx(1.0));
}

TEST_CASE("a zero-duration cooldown reports fully elapsed instead of dividing by zero") {
    Cooldown cooldown(1, Position{0, 0}, 0);
    CHECK(cooldown.fractionElapsed() == doctest::Approx(1.0));
}

TEST_CASE("a cooldown exposes the piece id and cell it was created with") {
    Cooldown cooldown(7, Position{2, 3}, 1000);
    CHECK(cooldown.pieceId() == 7);
    CHECK(cooldown.cell() == Position{2, 3});
}
