#pragma once
#include <vector>
#include <string>
#include <ostream>
#include <istream>
#include <optional>
#include "PieceType.h"

class Board {
public:
    static bool fromStream(std::istream& in, Board& outBoard, std::string& errorMessage);

    void print(std::ostream& out) const;
    bool inBounds(int row, int col) const;
    bool isEmpty(int row, int col) const;
    bool isSameColor(int row1, int col1, int row2, int col2) const;
    bool isPathClear(int fromRow, int fromCol, int toRow, int toCol) const;
    void movePiece(int fromRow, int fromCol, int toRow, int toCol);
    std::optional<PieceType> pieceTypeAt(int row, int col) const;

private:
    static bool isValidToken(const std::string& token);
    void rebuildRow(int row);

    std::vector<std::string> rows_;
    std::vector<std::vector<std::string>> grid_;
    
    int width_ = -1;
};