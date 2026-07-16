#include "input/BoardMapper.h"
#include <cmath>

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

PixelPoint BoardMapper::topLeftPixelOf(const Position& cell) const {
    return PixelPoint{cell.col * cellSizePixels_, cell.row * cellSizePixels_};
}

PixelPoint BoardMapper::topLeftPixelOf(const BoardPoint& point) const {
    return PixelPoint{static_cast<int>(std::lround(point.col * cellSizePixels_)),
                      static_cast<int>(std::lround(point.row * cellSizePixels_))};
}

int BoardMapper::cellSizePixels() const {
    return cellSizePixels_;
}

int BoardMapper::boardPixelWidth() const {
    return cols_ * cellSizePixels_;
}

int BoardMapper::boardPixelHeight() const {
    return rows_ * cellSizePixels_;
}
