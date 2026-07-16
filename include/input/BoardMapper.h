#pragma once
#include <optional>
#include "input/PixelPoint.h"
#include "model/BoardPoint.h"
#include "model/Position.h"

// Translates pixel coordinates into board cells — the only place in the
// codebase that knows a cell is CELL_SIZE pixels wide/tall. Knows nothing
// about pieces, selection, or move legality: it only answers "which cell,
// if any, is under this pixel."
class BoardMapper {
public:
    explicit BoardMapper(int rows = 0, int cols = 0, int cellSizePixels = 1);

    std::optional<Position> cellAt(int pixelX, int pixelY) const;
    PixelPoint topLeftPixelOf(const Position& cell) const;
    PixelPoint topLeftPixelOf(const BoardPoint& point) const;
    int cellSizePixels() const;
    int boardPixelWidth() const;
    int boardPixelHeight() const;

private:
    int rows_;
    int cols_;
    int cellSizePixels_;
};
