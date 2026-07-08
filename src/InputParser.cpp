#include "InputParser.h"
#include <sstream>
namespace {
    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
}
// Splits the input into board text and a list of command lines.
void InputParser::parse(std::istream& in, std::string& boardText, std::vector<std::string>& commandLines) {
    std::string line;
    std::ostringstream boardStream;
    bool readingCommands = false;

    while (std::getline(in, line)) {
        if (trim(line) == "Commands:") {
                readingCommands = true;
            continue;
        }
        if (readingCommands) {
            if (!line.empty()) commandLines.push_back(line);
        } else {
            boardStream << line << "\n";
        }
    }
    boardText = boardStream.str();
}