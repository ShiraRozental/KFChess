#include "model/BoardPoint.h"

BoardPoint toBoardPoint(const Position& cell) {
    return BoardPoint{static_cast<double>(cell.row), static_cast<double>(cell.col)};
}
