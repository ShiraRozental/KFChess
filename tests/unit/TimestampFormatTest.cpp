#include "doctest/doctest.h"
#include "logic/io/TimestampFormat.h"

TEST_CASE("zero formats as the zero clock") {
    CHECK(formatClockMs(0) == "00:00.000");
}

TEST_CASE("every field is zero-padded") {
    CHECK(formatClockMs(9021) == "00:09.021");
}

TEST_CASE("seconds roll over into minutes") {
    CHECK(formatClockMs(61500) == "01:01.500");
}

TEST_CASE("minutes beyond an hour keep counting without rolling over") {
    CHECK(formatClockMs(4500000) == "75:00.000");
}

TEST_CASE("a negative timestamp clamps to the zero clock") {
    CHECK(formatClockMs(-15) == "00:00.000");
}
