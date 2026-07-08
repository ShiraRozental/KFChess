#pragma once
#include <vector>
#include <string>
#include <ostream>
#include <istream>

class Board {
public:
    static bool fromStream(std::istream& in, Board& outBoard, std::string& errorMessage);

    void print(std::ostream& out) const;
    bool inBounds(int row, int col) const;
    const std::string& tokenAt(int row, int col) const;
    void movePiece(int fromRow, int fromCol, int toRow, int toCol);

private:
    static bool isValidToken(const std::string& token);
    void rebuildRow(int row);

    std::vector<std::string> rows_;
    std::vector<std::vector<std::string>> grid_;
    
    int width_ = -1;
};