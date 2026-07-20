#pragma once
#include <ostream>

// A single board cell — (row, col) — not a pixel coordinate. Pure value
// object: no bounds checking (that's Board's job), no knowledge of board
// size, movement rules, or rendering.
struct Position {
    int row;
    int col;
};

bool operator==(const Position& lhs, const Position& rhs);
bool operator!=(const Position& lhs, const Position& rhs);

// Row-major ordering (row first, then col), needed only so Position can be
// used as a std::set/std::map key. Carries no board-direction meaning.
bool operator<(const Position& lhs, const Position& rhs);

std::ostream& operator<<(std::ostream& out, const Position& position);
