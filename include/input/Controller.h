#pragma once
#include <optional>
#include "engine/IGameEngine.h"
#include "input/BoardMapper.h"
#include "model/Position.h"

// Result of a single click: whether it reached the point of requesting a
// move, and (only if so) what GameEngine said. moveResult is meaningless
// when requestedMove is false (first click, or any click outside the
// board) — there's nothing to report.
struct ControllerResult {
    bool requestedMove;
    MoveResult moveResult;
};

// Translates click input into game commands and owns the currently-
// selected cell. Never decides chess legality, never touches Board or
// RuleEngine directly — only IGameEngine (for the move/jump request itself
// and the hasPieceAt validation-only query) and BoardMapper (pixel->cell).
class Controller {
public:
    Controller(IGameEngine& engine, BoardMapper mapper);

    ControllerResult click(int pixelX, int pixelY);
    void jump(int pixelX, int pixelY);

    std::optional<Position> selectedCell() const;

private:
    IGameEngine& engine_;
    BoardMapper mapper_;
    std::optional<Position> selected_;
};
