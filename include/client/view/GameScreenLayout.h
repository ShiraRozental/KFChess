#pragma once
#include "client/input/PixelPoint.h"
#include "logic/model/PieceColor.h"
#include "client/view/PixelSize.h"

struct PixelRect {
    int x;
    int y;
    int width;
    int height;
};

class GameScreenLayout {
public:
    GameScreenLayout(int rows, int cols, int cellSizePixels);

    // Largest cell size whose resulting window still fits in `available`.
    static int fitCellSize(int rows, int cols, PixelSize available);

    int windowWidth() const;
    int windowHeight() const;
    PixelPoint boardOrigin() const;
    PixelRect movesPanelRect(PieceColor color) const;
    PixelRect bannerRect(PieceColor color) const;
    int labelMargin() const;
    int logLineHeight() const;
    int maxLogLines() const;

private:
    int boardWidth_;
    int boardHeight_;
    int labelMargin_;
    int panelWidth_;
    int panelGap_;
    int bannerHeight_;
    int logLineHeight_;
};
