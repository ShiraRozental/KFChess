#pragma once
#include "view/Renderer.h"

// Placeholder graphical Renderer: owns just enough to know the board's
// pixel geometry (board size, cell size), mirroring what BoardMapper knows
// for input. render() does not draw anything yet — no graphics library has
// been chosen for this project. Once one is, this class becomes the seam
// where sprite loading and drawing are implemented, with no change needed
// to Renderer, GameEngine, or GameSnapshot.
class ImageView : public Renderer {
public:
    ImageView(int rows, int cols, int cellSizePixels);

    void render(const GameSnapshot& snapshot) override;

private:
    int rows_;
    int cols_;
    int cellSizePixels_;
};
