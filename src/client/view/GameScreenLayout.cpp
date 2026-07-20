#include "client/view/GameScreenLayout.h"
#include <algorithm>
#include <cmath>

namespace {
    constexpr double kLabelMarginRatio = 0.45;
    constexpr double kPanelWidthRatio = 2.2;
    constexpr double kPanelGapRatio = 0.25;
    constexpr double kBannerHeightRatio = 0.9;
    constexpr double kLogLineHeightRatio = 0.35;
    constexpr int kLogHeaderLines = 1;
    constexpr int kPreferredCellSize = 100;
    constexpr int kMinCellSize = 24;

    int scaled(int cellSizePixels, double ratio) {
        return static_cast<int>(std::lround(cellSizePixels * ratio));
    }
}

GameScreenLayout::GameScreenLayout(int rows, int cols, int cellSizePixels)
    : boardWidth_(cols * cellSizePixels),
      boardHeight_(rows * cellSizePixels),
      labelMargin_(scaled(cellSizePixels, kLabelMarginRatio)),
      panelWidth_(scaled(cellSizePixels, kPanelWidthRatio)),
      panelGap_(scaled(cellSizePixels, kPanelGapRatio)),
      bannerHeight_(scaled(cellSizePixels, kBannerHeightRatio)),
      logLineHeight_(scaled(cellSizePixels, kLogLineHeightRatio)) {
}

// The chrome ratios make window size linear in cell size, so the closed form
// lands on or just above the answer; rounding is settled by stepping down.
int GameScreenLayout::fitCellSize(int rows, int cols, PixelSize available) {
    const GameScreenLayout unitLayout(rows, cols, kPreferredCellSize);
    const double widthLimit =
        static_cast<double>(available.width) * kPreferredCellSize / unitLayout.windowWidth();
    const double heightLimit =
        static_cast<double>(available.height) * kPreferredCellSize / unitLayout.windowHeight();

    int cellSize = std::min(kPreferredCellSize,
                            static_cast<int>(std::floor(std::min(widthLimit, heightLimit))));
    while (cellSize > kMinCellSize) {
        const GameScreenLayout candidate(rows, cols, cellSize);
        if (candidate.windowWidth() <= available.width &&
            candidate.windowHeight() <= available.height) {
            break;
        }
        --cellSize;
    }
    return std::max(cellSize, kMinCellSize);
}

int GameScreenLayout::windowWidth() const {
    return 2 * (panelWidth_ + panelGap_ + labelMargin_) + boardWidth_;
}

int GameScreenLayout::windowHeight() const {
    return 2 * (bannerHeight_ + labelMargin_) + boardHeight_;
}

PixelPoint GameScreenLayout::boardOrigin() const {
    return PixelPoint{panelWidth_ + panelGap_ + labelMargin_,
                      bannerHeight_ + labelMargin_};
}

PixelRect GameScreenLayout::movesPanelRect(PieceColor color) const {
    int x = color == PieceColor::Black ? 0 : windowWidth() - panelWidth_;
    return PixelRect{x, boardOrigin().y, panelWidth_, boardHeight_};
}

PixelRect GameScreenLayout::bannerRect(PieceColor color) const {
    int y = color == PieceColor::Black ? 0 : windowHeight() - bannerHeight_;
    return PixelRect{0, y, windowWidth(), bannerHeight_};
}

int GameScreenLayout::labelMargin() const {
    return labelMargin_;
}

int GameScreenLayout::logLineHeight() const {
    return logLineHeight_;
}

int GameScreenLayout::maxLogLines() const {
    return std::max(0, boardHeight_ / logLineHeight_ - kLogHeaderLines);
}
