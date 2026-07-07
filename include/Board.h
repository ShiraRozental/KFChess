#pragma once
#include <vector>
#include <string>
#include <ostream>
#include <istream>

class Board {
public:
    static bool fromStream(std::istream& in, Board& outBoard, std::string& errorMessage);

    void print(std::ostream& out) const;

private:
    static bool isValidToken(const std::string& token);

    std::vector<std::string> rows_;
    int width_ = -1;
};