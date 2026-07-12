#pragma once

// Pure geometric legality check for a piece's movement shape. Implementations
// only reason about row/column deltas — no board state, occupancy, capture,
// or path-blocking (sliding pieces jumping over other pieces is a future
// iteration, out of scope here).
class MovementRule {
public:
    virtual ~MovementRule() = default;
    virtual bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const = 0;

    // Whether this piece's movement is blocked by other pieces standing on
    // the straight/diagonal line between source and destination (rook,
    // bishop, queen). Pieces that move without regard to what's in between
    // (king, knight) keep the default of false.
    virtual bool requiresClearPath() const { return false; }
};
