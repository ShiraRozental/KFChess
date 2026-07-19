#include "doctest/doctest.h"
#include "view/GameScreenLayout.h"

namespace {
    const GameScreenLayout layout(8, 8, 100);
    constexpr int kBoardSidePixels = 800;
    constexpr int kPreferredCellSize = 100;
    constexpr int kMinCellSize = 24;

    bool fitsIn(int rows, int cols, int cellSize, PixelSize available) {
        const GameScreenLayout candidate(rows, cols, cellSize);
        return candidate.windowWidth() <= available.width &&
               candidate.windowHeight() <= available.height;
    }
}

TEST_CASE("the black panel hugs the left edge and the white panel the right edge") {
    PixelRect black = layout.movesPanelRect(PieceColor::Black);
    PixelRect white = layout.movesPanelRect(PieceColor::White);

    CHECK(black.x == 0);
    CHECK(white.x + white.width == layout.windowWidth());
}

TEST_CASE("the board is centered between the side panels") {
    PixelRect black = layout.movesPanelRect(PieceColor::Black);
    PixelRect white = layout.movesPanelRect(PieceColor::White);
    PixelPoint origin = layout.boardOrigin();

    int leftSpace = origin.x - (black.x + black.width);
    int rightSpace = white.x - (origin.x + kBoardSidePixels);
    CHECK(leftSpace > 0);
    CHECK(leftSpace == rightSpace);
}

TEST_CASE("the side panels align vertically with the board") {
    PixelRect panel = layout.movesPanelRect(PieceColor::White);
    CHECK(panel.y == layout.boardOrigin().y);
    CHECK(panel.height == kBoardSidePixels);
}

TEST_CASE("the banners span the full window width at the top and bottom") {
    PixelRect black = layout.bannerRect(PieceColor::Black);
    PixelRect white = layout.bannerRect(PieceColor::White);

    CHECK(black.y == 0);
    CHECK(black.width == layout.windowWidth());
    CHECK(white.y + white.height == layout.windowHeight());
    CHECK(white.width == layout.windowWidth());
}

TEST_CASE("the board fits inside the window with label margins around it") {
    PixelPoint origin = layout.boardOrigin();
    PixelRect topBanner = layout.bannerRect(PieceColor::Black);

    CHECK(origin.y - (topBanner.y + topBanner.height) == layout.labelMargin());
    CHECK(origin.y + kBoardSidePixels + layout.labelMargin() <= layout.windowHeight());
    CHECK(origin.x + kBoardSidePixels + layout.labelMargin() <= layout.windowWidth());
}

TEST_CASE("the log lines plus a header line fit inside the panel height") {
    int usedHeight = (layout.maxLogLines() + 1) * layout.logLineHeight();
    CHECK(layout.maxLogLines() > 0);
    CHECK(usedHeight <= layout.movesPanelRect(PieceColor::Black).height);
}

TEST_CASE("a roomy screen keeps the preferred cell size instead of growing") {
    CHECK(GameScreenLayout::fitCellSize(8, 8, PixelSize{5000, 5000}) == kPreferredCellSize);
}

TEST_CASE("a screen limited by height shrinks the cells until the window fits") {
    const PixelSize available{1528, 785};
    int cellSize = GameScreenLayout::fitCellSize(8, 8, available);

    CHECK(cellSize < kPreferredCellSize);
    CHECK(fitsIn(8, 8, cellSize, available));
}

TEST_CASE("a screen limited by width shrinks the cells until the window fits") {
    const PixelSize available{700, 2000};
    int cellSize = GameScreenLayout::fitCellSize(8, 8, available);

    CHECK(cellSize < kPreferredCellSize);
    CHECK(fitsIn(8, 8, cellSize, available));
}

TEST_CASE("the chosen cell size is the largest one that still fits") {
    const PixelSize available{1528, 785};
    int cellSize = GameScreenLayout::fitCellSize(8, 8, available);

    CHECK(fitsIn(8, 8, cellSize, available));
    CHECK_FALSE(fitsIn(8, 8, cellSize + 1, available));
}

TEST_CASE("a non-square board respects both axes at once") {
    const PixelSize available{900, 900};
    int cellSize = GameScreenLayout::fitCellSize(10, 4, available);

    CHECK(fitsIn(10, 4, cellSize, available));
    CHECK_FALSE(fitsIn(10, 4, cellSize + 1, available));
}

TEST_CASE("a tiny screen falls back to the minimum cell size rather than vanishing") {
    CHECK(GameScreenLayout::fitCellSize(8, 8, PixelSize{1, 1}) == kMinCellSize);
}

TEST_CASE("more available space never yields a smaller cell size") {
    int previous = 0;
    for (int side = 200; side <= 2000; side += 50) {
        int cellSize = GameScreenLayout::fitCellSize(8, 8, PixelSize{side, side});
        CHECK(cellSize >= previous);
        previous = cellSize;
    }
}
