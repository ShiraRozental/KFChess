#include "Board.h"
#include <sstream>

bool Board::isValidToken(const std::string& token) {
    if (token == ".") return true;
    if (token.length() != 2) return false;
    char color = token[0];
    char piece = token[1];
    if (color != 'w' && color != 'b') return false;
    return std::string("KQRBNP").find(piece) != std::string::npos;
}

bool Board::fromStream(std::istream& in, Board& outBoard, std::string& errorMessage) {
    std::string line;
    bool readingBoard = false;

    while (std::getline(in, line)) {
        if (line == "Board:") { readingBoard = true; continue; }
        if (line == "Commands:") { readingBoard = false; continue; }
        if (!readingBoard || line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        int currentWidth = 0;

        while (ss >> token) {
            if (!isValidToken(token)) {
                errorMessage = "ERROR UNKNOWN_TOKEN";
                return false;
            }
            currentWidth++;
        }

        if (outBoard.width_ == -1) {
            outBoard.width_ = currentWidth;
        } else if (currentWidth != outBoard.width_) {
            errorMessage = "ERROR ROW_WIDTH_MISMATCH";
            return false;
        }

        outBoard.rows_.push_back(line);
    }
    return true;
}

void Board::print(std::ostream& out) const {
    for (size_t i = 0; i < rows_.size(); ++i) {
        out << rows_[i];
        if (i < rows_.size() - 1) out << "\n";
    }
}