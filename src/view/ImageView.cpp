#include "view/ImageView.h"

ImageView::ImageView(int rows, int cols, int cellSizePixels)
    : rows_(rows), cols_(cols), cellSizePixels_(cellSizePixels) {
}

// Nothing to draw yet: no graphics library is wired into this project (see
// CLAUDE.md — business logic only, no graphics yet). This is the single
// place a future implementation fills in once one is chosen.
void ImageView::render(const GameSnapshot&) {
}
