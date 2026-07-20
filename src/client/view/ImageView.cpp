#include "client/view/ImageView.h"
#include <cmath>
#include <set>
#include "logic/io/PieceNotation.h"
#include "logic/io/TimestampFormat.h"

namespace {
    constexpr int kShowWaitMs = 1;

    constexpr double kPieceMarginRatio = 0.1;
    constexpr double kCooldownBarHeightRatio = 0.08;
    constexpr double kSelectionBorderRatio = 0.06;
    constexpr double kDestinationMarkerRatio = 0.22;
    const cv::Scalar kCooldownBarBackground(60, 60, 60, 255);
    const cv::Scalar kCooldownBarFill(80, 200, 80, 255);
    const cv::Scalar kSelectionColor(0, 215, 255, 255);

    const cv::Scalar kWindowBackground(45, 42, 38);
    const cv::Scalar kPanelBackground(28, 26, 24);
    const cv::Scalar kTextColor(225, 225, 225, 255);
    constexpr double kLabelTextRatio = 0.0045;
    constexpr double kPanelTextRatio = 0.004;
    constexpr double kBannerTextRatio = 0.006;
    constexpr int kTextThickness = 1;
    constexpr double kMoveColumnRatio = 0.55;
    constexpr double kPanelPaddingRatio = 0.15;
    constexpr double kLineBaselineRatio = 0.75;
    constexpr double kNameBaselineRatio = 0.45;
    constexpr double kScoreBaselineRatio = 0.85;
    constexpr const char* kTimeColumnHeader = "Time";
    constexpr const char* kMoveColumnHeader = "Move";
    constexpr const char* kNamePrefix = "Name: ";
    constexpr const char* kScorePrefix = "Score: ";
    constexpr const char* kWinsSuffix = " wins!";
    constexpr const char* kNoWinnerText = "Game over";
    const cv::Scalar kGameOverStripColor(20, 20, 20);
    constexpr double kGameOverStripHeightRatio = 1.2;
    constexpr double kGameOverTextRatio = 0.012;
    constexpr int kGameOverTextThickness = 2;

    int pieceMarginFor(int cellSizePixels) {
        return static_cast<int>(std::lround(cellSizePixels * kPieceMarginRatio));
    }

    bool isResting(Piece::State state) {
        return state == Piece::State::ShortRest || state == Piece::State::LongRest;
    }

    int scaled(int base, double ratio) {
        return static_cast<int>(std::lround(base * ratio));
    }

    int logLineBaseline(const PixelRect& panel, int lineIndex, int lineHeight) {
        return panel.y + lineIndex * lineHeight + scaled(lineHeight, kLineBaselineRatio);
    }
}

ImageView::ImageView(BoardMapper mapper, GameScreenLayout layout,
                     const MovesLog& movesLog, const ScoreBoard& scoreBoard,
                     PlayerNames playerNames, const std::filesystem::path& assetsRoot,
                     const std::filesystem::path& boardImagePath)
    : mapper_(mapper),
      layout_(layout),
      movesLog_(movesLog),
      scoreBoard_(scoreBoard),
      playerNames_(std::move(playerNames)),
      pieceMarginPixels_(pieceMarginFor(mapper.cellSizePixels())),
      cache_(assetsRoot, mapper.cellSizePixels() - 2 * pieceMarginPixels_),
      background_(buildBackground(boardImagePath)) {
}

void ImageView::drawCenteredText(Img& target, const std::string& text, int centerX,
                                 int baselineY, double fontScale, int thickness) {
    int textWidth = Img::text_width(text, fontScale, thickness);
    target.put_text(text, centerX - textWidth / 2, baselineY, fontScale,
                    kTextColor, thickness);
}

void ImageView::drawBoardLabels(Img& canvas) {
    PixelPoint origin = layout_.boardOrigin();
    int cell = mapper_.cellSizePixels();
    int boardWidth = mapper_.boardPixelWidth();
    int boardHeight = mapper_.boardPixelHeight();
    int margin = layout_.labelMargin();
    double fontScale = cell * kLabelTextRatio;

    for (int col = 0; col < boardWidth / cell; ++col) {
        std::string label = fileLabel(col);
        int centerX = origin.x + col * cell + cell / 2;
        drawCenteredText(canvas, label, centerX, origin.y - margin / 3, fontScale,
                         kTextThickness);
        drawCenteredText(canvas, label, centerX,
                         origin.y + boardHeight + (2 * margin) / 3, fontScale,
                         kTextThickness);
    }

    int rowCount = boardHeight / cell;
    for (int row = 0; row < rowCount; ++row) {
        std::string label = rankLabel(row, rowCount);
        int baselineY = origin.y + row * cell + cell / 2 + margin / 3;
        drawCenteredText(canvas, label, origin.x - margin / 2, baselineY, fontScale,
                         kTextThickness);
        drawCenteredText(canvas, label, origin.x + boardWidth + margin / 2, baselineY,
                         fontScale, kTextThickness);
    }
}

Img ImageView::buildBackground(const std::filesystem::path& boardImagePath) {
    Img canvas;
    canvas.create(layout_.windowWidth(), layout_.windowHeight(), kWindowBackground);

    Img board;
    board.read(boardImagePath, {mapper_.boardPixelWidth(), mapper_.boardPixelHeight()});
    PixelPoint origin = layout_.boardOrigin();
    board.draw_on(canvas, origin.x, origin.y);

    drawBoardLabels(canvas);

    int cell = mapper_.cellSizePixels();
    int padding = scaled(cell, kPanelPaddingRatio);
    double fontScale = cell * kPanelTextRatio;
    for (PieceColor color : {PieceColor::White, PieceColor::Black}) {
        PixelRect panel = layout_.movesPanelRect(color);
        canvas.fill_rect(panel.x, panel.y, panel.width, panel.height, kPanelBackground);
        int headerBaseline = logLineBaseline(panel, 0, layout_.logLineHeight());
        canvas.put_text(kTimeColumnHeader, panel.x + padding, headerBaseline,
                        fontScale, kTextColor, kTextThickness);
        canvas.put_text(kMoveColumnHeader, panel.x + scaled(panel.width, kMoveColumnRatio),
                        headerBaseline, fontScale, kTextColor, kTextThickness);
    }
    return canvas;
}

void ImageView::render(const GameSnapshot& snapshot) {
    long long dtMs = elapsedSinceLastRenderMs();
    Img frame = background_.clone();
    drawSelectionHighlight(frame);
    drawLegalDestinationMarkers(frame);
    drawPieces(snapshot, frame, dtMs);
    for (PieceColor color : {PieceColor::White, PieceColor::Black}) {
        drawMovesPanel(frame, color);
        drawPlayerBanner(frame, color);
    }
    if (snapshot.isGameOver()) {
        drawGameOverOverlay(frame, snapshot.winner());
    }
    removeViewsOfCapturedPieces(snapshot.board());
    frame.show(kShowWaitMs);
}

void ImageView::drawGameOverOverlay(Img& frame, std::optional<PieceColor> winner) {
    int cell = mapper_.cellSizePixels();
    int stripHeight = scaled(cell, kGameOverStripHeightRatio);
    int stripY = layout_.boardOrigin().y + (mapper_.boardPixelHeight() - stripHeight) / 2;
    frame.fill_rect(0, stripY, layout_.windowWidth(), stripHeight, kGameOverStripColor);

    std::string message = winner ? playerNames_.of(*winner) + kWinsSuffix : kNoWinnerText;
    int centerX = layout_.windowWidth() / 2;
    int baselineY = stripY + scaled(stripHeight, kLineBaselineRatio);
    drawCenteredText(frame, message, centerX, baselineY, cell * kGameOverTextRatio,
                     kGameOverTextThickness);
}

void ImageView::drawMovesPanel(Img& frame, PieceColor color) {
    PixelRect panel = layout_.movesPanelRect(color);
    int cell = mapper_.cellSizePixels();
    int padding = scaled(cell, kPanelPaddingRatio);
    double fontScale = cell * kPanelTextRatio;
    int lineHeight = layout_.logLineHeight();

    std::vector<MoveLogEntry> entries = movesLog_.lastEntriesOf(color, layout_.maxLogLines());
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        int baseline = logLineBaseline(panel, i + 1, lineHeight);
        frame.put_text(formatClockMs(entries[i].timestampMs), panel.x + padding,
                       baseline, fontScale, kTextColor, kTextThickness);
        frame.put_text(entries[i].text, panel.x + scaled(panel.width, kMoveColumnRatio),
                       baseline, fontScale, kTextColor, kTextThickness);
    }
}

void ImageView::drawPlayerBanner(Img& frame, PieceColor color) {
    PixelRect banner = layout_.bannerRect(color);
    double fontScale = mapper_.cellSizePixels() * kBannerTextRatio;
    int centerX = banner.x + banner.width / 2;

    drawCenteredText(frame, kNamePrefix + playerNames_.of(color), centerX,
                     banner.y + scaled(banner.height, kNameBaselineRatio), fontScale,
                     kTextThickness);
    drawCenteredText(frame, kScorePrefix + std::to_string(scoreBoard_.scoreOf(color)),
                     centerX, banner.y + scaled(banner.height, kScoreBaselineRatio),
                     fontScale, kTextThickness);
}

long long ImageView::elapsedSinceLastRenderMs() {
    auto now = std::chrono::steady_clock::now();
    long long dtMs = 0;
    if (lastRenderTime_) {
        dtMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - *lastRenderTime_).count();
    }
    lastRenderTime_ = now;
    return dtMs;
}

void ImageView::setSelectedCell(std::optional<Position> cell) {
    selectedCell_ = cell;
}

void ImageView::setLegalDestinations(std::set<Position> destinations) {
    legalDestinations_ = std::move(destinations);
}

// A frame of four thin strips along the selected cell's edges, drawn under
// the pieces; the piece margin keeps sprites clear of it anyway.
void ImageView::drawSelectionHighlight(Img& frame) {
    if (!selectedCell_) return;

    PixelPoint corner = mapper_.topLeftPixelOf(*selectedCell_);
    int cellSize = mapper_.cellSizePixels();
    int thickness = std::max(1, static_cast<int>(std::lround(cellSize * kSelectionBorderRatio)));

    frame.fill_rect(corner.x, corner.y, cellSize, thickness, kSelectionColor);
    frame.fill_rect(corner.x, corner.y + cellSize - thickness, cellSize, thickness, kSelectionColor);
    frame.fill_rect(corner.x, corner.y + thickness, thickness, cellSize - 2 * thickness, kSelectionColor);
    frame.fill_rect(corner.x + cellSize - thickness, corner.y + thickness, thickness,
                    cellSize - 2 * thickness, kSelectionColor);
}

void ImageView::drawLegalDestinationMarkers(Img& frame) {
    int cellSize = mapper_.cellSizePixels();
    int markerSize = static_cast<int>(std::lround(cellSize * kDestinationMarkerRatio));
    int centerOffset = (cellSize - markerSize) / 2;

    for (const Position& destination : legalDestinations_) {
        PixelPoint corner = mapper_.topLeftPixelOf(destination);
        frame.fill_rect(corner.x + centerOffset, corner.y + centerOffset,
                        markerSize, markerSize, kSelectionColor);
    }
}

PieceView& ImageView::viewFor(const Piece& piece) {
    auto found = pieceViews_.find(piece.id());
    if (found == pieceViews_.end() || found->second.kind != piece.kind()) {
        const auto& animations = cache_.animationsFor(piece.color(), piece.kind());
        TrackedPiece tracked{piece.kind(), PieceView(animations, piece.state())};
        found = pieceViews_.insert_or_assign(piece.id(), std::move(tracked)).first;
    }
    return found->second.view;
}

void ImageView::drawPieces(const GameSnapshot& snapshot, Img& frame, long long dtMs) {
    const Board& board = snapshot.board();
    for (int row = 0; row < board.rowCount(); ++row) {
        for (int col = 0; col < board.colCount(); ++col) {
            const Piece* piece = board.pieceAt(row, col);
            if (!piece) continue;

            PieceView& view = viewFor(*piece);
            view.advance(piece->state(), dtMs);
            std::optional<BoardPoint> inFlight = snapshot.inFlightPositionOf(piece->id());
            PixelPoint pixel = inFlight ? mapper_.topLeftPixelOf(*inFlight)
                                        : mapper_.topLeftPixelOf(piece->cell());
            const Img& sprite = view.currentFrame();
            int cellSize = mapper_.cellSizePixels();
            sprite.draw_on(frame,
                           pixel.x + (cellSize - sprite.width()) / 2,
                           pixel.y + (cellSize - sprite.height()) / 2);

            std::optional<double> progress = snapshot.cooldownProgressOf(piece->id());
            if (isResting(piece->state()) && progress) {
                drawCooldownBar(frame, pixel, *progress);
            }
        }
    }
}

// A two-layer horizontal bar along the bottom edge of the cell: a dark
// track over the full width, and a fill growing left-to-right with progress.
void ImageView::drawCooldownBar(Img& frame, PixelPoint cellTopLeft, double progress) {
    int cellSize = mapper_.cellSizePixels();
    int barWidth = cellSize - 2 * pieceMarginPixels_;
    int barHeight = static_cast<int>(std::lround(cellSize * kCooldownBarHeightRatio));
    int barX = cellTopLeft.x + pieceMarginPixels_;
    int barY = cellTopLeft.y + cellSize - barHeight;
    int fillWidth = static_cast<int>(std::lround(barWidth * progress));

    frame.fill_rect(barX, barY, barWidth, barHeight, kCooldownBarBackground);
    frame.fill_rect(barX, barY, fillWidth, barHeight, kCooldownBarFill);
}

void ImageView::removeViewsOfCapturedPieces(const Board& board) {
    std::set<PieceId> alive;
    for (int row = 0; row < board.rowCount(); ++row) {
        for (int col = 0; col < board.colCount(); ++col) {
            const Piece* piece = board.pieceAt(row, col);
            if (piece) alive.insert(piece->id());
        }
    }

    for (auto it = pieceViews_.begin(); it != pieceViews_.end();) {
        it = alive.count(it->first) ? std::next(it) : pieceViews_.erase(it);
    }
}
