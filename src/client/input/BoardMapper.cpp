#include "client/input/BoardMapper.h"
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

BoardMapper::BoardMapper(int rows, int cols, int cellSizePixels, PixelPoint origin)
    : rows_(rows), cols_(cols), cellSizePixels_(cellSizePixels), origin_(origin) {
}

std::optional<Position> BoardMapper::cellAt(int pixelX, int pixelY) const {
    int col = pixelToCell(pixelX - origin_.x, cellSizePixels_);
    int row = pixelToCell(pixelY - origin_.y, cellSizePixels_);

    if (row < 0 || row >= rows_ || col < 0 || col >= cols_) return std::nullopt;
    return Position{row, col};
}

PixelPoint BoardMapper::topLeftPixelOf(const Position& cell) const {
    return PixelPoint{origin_.x + cell.col * cellSizePixels_,
                      origin_.y + cell.row * cellSizePixels_};
}

PixelPoint BoardMapper::topLeftPixelOf(const BoardPoint& point) const {
    return PixelPoint{origin_.x + static_cast<int>(std::lround(point.col * cellSizePixels_)),
                      origin_.y + static_cast<int>(std::lround(point.row * cellSizePixels_))};
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
