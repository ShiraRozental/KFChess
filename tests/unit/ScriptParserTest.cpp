#include "doctest/doctest.h"
#include "texttests/ScriptParser.h"

TEST_CASE("a click command parses its x and y") {
    std::optional<Command> command = parseCommand("click 50 150");
    REQUIRE(command.has_value());
    REQUIRE(std::holds_alternative<ClickCommand>(*command));
    CHECK(std::get<ClickCommand>(*command).x == 50);
    CHECK(std::get<ClickCommand>(*command).y == 150);
}

TEST_CASE("a jump command parses its x and y, including negative values") {
    std::optional<Command> command = parseCommand("jump -10 50");
    REQUIRE(command.has_value());
    REQUIRE(std::holds_alternative<JumpCommand>(*command));
    CHECK(std::get<JumpCommand>(*command).x == -10);
    CHECK(std::get<JumpCommand>(*command).y == 50);
}

TEST_CASE("a wait command parses its duration") {
    std::optional<Command> command = parseCommand("wait 1000");
    REQUIRE(command.has_value());
    REQUIRE(std::holds_alternative<WaitCommand>(*command));
    CHECK(std::get<WaitCommand>(*command).ms == 1000);
}

TEST_CASE("a print board command parses with no further arguments") {
    std::optional<Command> command = parseCommand("print board");
    REQUIRE(command.has_value());
    CHECK(std::holds_alternative<PrintBoardCommand>(*command));
}

TEST_CASE("a click command with non-numeric arguments is rejected") {
    CHECK_FALSE(parseCommand("click abc def").has_value());
}

TEST_CASE("a click command with a missing argument is rejected") {
    CHECK_FALSE(parseCommand("click 50").has_value());
}

TEST_CASE("an unknown keyword is rejected") {
    CHECK_FALSE(parseCommand("dance 1 2").has_value());
}

TEST_CASE("print with anything other than board is rejected") {
    CHECK_FALSE(parseCommand("print score").has_value());
}

TEST_CASE("a blank line is rejected") {
    CHECK_FALSE(parseCommand("").has_value());
}
