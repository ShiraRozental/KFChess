#pragma once

// Pure geometric legality check for a piece's movement shape. Implementations
// only reason about row/column deltas — no board state, occupancy, capture,
// or path-blocking (sliding pieces jumping over other pieces is a future
// iteration, out of scope here).
class MovementRule {
public:
    virtual ~MovementRule() = default;
    virtual bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const = 0;
};
