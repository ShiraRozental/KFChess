#pragma once
#include <istream>
#include <string>
#include <vector>
#include "logic/model/Board.h"

struct ParsedInput {
    Board board;
    std::vector<std::string> commands;
};

class BoardParser {
public:
    static bool parse(std::istream& in, ParsedInput& out, std::string& errorMessage);
};
