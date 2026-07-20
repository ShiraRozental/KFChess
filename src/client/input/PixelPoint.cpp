#include "client/input/PixelPoint.h"

bool operator==(const PixelPoint& lhs, const PixelPoint& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const PixelPoint& lhs, const PixelPoint& rhs) {
    return !(lhs == rhs);
}
