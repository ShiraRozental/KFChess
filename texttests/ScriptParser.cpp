#include "texttests/ScriptParser.h"
#include "logic/io/StringUtils.h"
#include <cstdlib>

namespace {
    std::optional<int> toInt(const std::string& s) {
        if (s.empty()) return std::nullopt;
        char* end = nullptr;
        long value = std::strtol(s.c_str(), &end, 10);
        if (end != s.c_str() + s.size()) return std::nullopt;
        return static_cast<int>(value);
    }
}

std::optional<Command> parseCommand(const std::string& line) {
    std::vector<std::string> tokens = tokenize(line);
    if (tokens.empty()) return std::nullopt;

    const std::string& keyword = tokens[0];

    if (keyword == "click" && tokens.size() == 3) {
        std::optional<int> x = toInt(tokens[1]);
        std::optional<int> y = toInt(tokens[2]);
        if (x && y) return ClickCommand{*x, *y};
        return std::nullopt;
    }
    if (keyword == "jump" && tokens.size() == 3) {
        std::optional<int> x = toInt(tokens[1]);
        std::optional<int> y = toInt(tokens[2]);
        if (x && y) return JumpCommand{*x, *y};
        return std::nullopt;
    }
    if (keyword == "wait" && tokens.size() == 2) {
        std::optional<int> ms = toInt(tokens[1]);
        if (ms) return WaitCommand{*ms};
        return std::nullopt;
    }
    if (keyword == "print" && tokens.size() == 2 && tokens[1] == "board") {
        return PrintBoardCommand{};
    }
    return std::nullopt;
}
