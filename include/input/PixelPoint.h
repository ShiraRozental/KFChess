#pragma once

struct PixelPoint {
    int x;
    int y;
};

bool operator==(const PixelPoint& lhs, const PixelPoint& rhs);
bool operator!=(const PixelPoint& lhs, const PixelPoint& rhs);
