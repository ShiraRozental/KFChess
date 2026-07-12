#pragma once
#include <istream>
#include <string>
#include <vector>

// Splits stdin into the "Board:" text block and the raw command lines.
// No game logic here — pure input/output boundary.
class BoardParser {
public:
    static void parse(std::istream& in, std::string& boardText, std::vector<std::string>& commandLines);
};