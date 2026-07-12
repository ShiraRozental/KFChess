#include "model/Position.h"

bool operator==(const Position& lhs, const Position& rhs) {
    return lhs.row == rhs.row && lhs.col == rhs.col;
}

bool operator!=(const Position& lhs, const Position& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Position& position) {
    return out << "(" << position.row << ", " << position.col << ")";
}
