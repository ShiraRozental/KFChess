#include "doctest/doctest.h"
#include "view/GameScreenLayout.h"

namespace {
    const GameScreenLayout layout(8, 8, 100);
    constexpr int kBoardSidePixels = 800;
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
