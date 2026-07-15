#include "input/BoardMapper.h"

namespace {
    // Floor division, so pixels left of/above the board (negative) map to
    // negative cell indices instead of wrapping to 0 under C++'s truncating
    // integer division.
    int pixelToCell(int pixel, int cellSizePixels) {
        if (pixel >= 0) return pixel / cellSizePixels;
        return (pixel - (cellSizePixels - 1)) / cellSizePixels;
    }
}

BoardMapper::BoardMapper(int rows, int cols, int cellSizePixels)
    : rows_(rows), cols_(cols), cellSizePixels_(cellSizePixels) {
}

std::optional<Position> BoardMapper::cellAt(int pixelX, int pixelY) const {
    int col = pixelToCell(pixelX, cellSizePixels_);
    int row = pixelToCell(pixelY, cellSizePixels_);

    if (row < 0 || row >= rows_ || col < 0 || col >= cols_) return std::nullopt;
    return Position{row, col};
}
