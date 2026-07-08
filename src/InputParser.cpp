#include "InputParser.h"
#include "StringUtils.h"
#include <sstream>
// Splits the input into board text and a list of command lines.
void InputParser::parse(std::istream& in, std::string& boardText, std::vector<std::string>& commandLines) {
    std::string line;
    std::ostringstream boardStream;
    bool readingCommands = false;

    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed == "Commands:") {
            readingCommands = true;
            continue;
        }
        if (readingCommands) {
            if (!trimmed.empty()) commandLines.push_back(line);
        } else {
            boardStream << line << "\n";
        }
    }
    boardText = boardStream.str();
}