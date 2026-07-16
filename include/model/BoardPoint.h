#pragma once
#include "model/Position.h"

struct BoardPoint {
    double row;
    double col;
};

BoardPoint toBoardPoint(const Position& cell);
