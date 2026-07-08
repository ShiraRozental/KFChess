#include "doctest/doctest.h"
#include "InputParser.h"
#include <sstream>

TEST_CASE("InputParser splits board text and command lines") {
    std::istringstream in("Board:\nwK .\n. .\nCommands:\nclick 0 0\nprint board\n");
    std::string boardText;
    std::vector<std::string> commands;
    InputParser::parse(in, boardText, commands);
    CHECK(boardText == "Board:\nwK .\n. .\n");
    REQUIRE(commands.size() == 2);
    CHECK(commands[0] == "click 0 0");
    CHECK(commands[1] == "print board");
}

TEST_CASE("InputParser skips blank and whitespace-only command lines") {
    std::istringstream in("Board:\nwK .\nCommands:\n   \nclick 0 0\n");
    std::string boardText;
    std::vector<std::string> commands;
    InputParser::parse(in, boardText, commands);
    REQUIRE(commands.size() == 1);
    CHECK(commands[0] == "click 0 0");
}
