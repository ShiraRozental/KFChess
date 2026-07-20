#include "client/input/Controller.h"

Controller::Controller(IGameEngine& engine, BoardMapper mapper)
    : engine_(engine), mapper_(mapper) {
}

// First click on an occupied cell selects it; on an empty cell, nothing
// happens. Any click outside the board clears the selection (if any) and
// never reaches GameEngine. A second click inside the board always
// requests a move and always clears the selection, whether the move is
// accepted or not.
ControllerResult Controller::click(int pixelX, int pixelY) {
    std::optional<Position> cell = mapper_.cellAt(pixelX, pixelY);

    if (!cell.has_value()) {
        selected_.reset();
        return ControllerResult{false, MoveResult{}};
    }

    if (!selected_.has_value()) {
        if (engine_.hasPieceAt(*cell)) selected_ = cell;
        return ControllerResult{false, MoveResult{}};
    }

    MoveResult result = engine_.requestMove(*selected_, *cell);
    selected_.reset();
    return ControllerResult{true, result};
}

// Pixel -> cell -> GameEngine::requestJump, independent of any selection.
void Controller::jump(int pixelX, int pixelY) {
    std::optional<Position> cell = mapper_.cellAt(pixelX, pixelY);
    if (!cell.has_value()) return;
    engine_.requestJump(*cell);
}

std::optional<Position> Controller::selectedCell() const {
    return selected_;
}
