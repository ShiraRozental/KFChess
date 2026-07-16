#include "view/ImageView.h"
#include <cmath>
#include <set>

namespace {
    constexpr int kShowWaitMs = 1;

    constexpr double kPieceMarginRatio = 0.1;
    constexpr double kCooldownBarHeightRatio = 0.08;
    constexpr double kSelectionBorderRatio = 0.06;
    constexpr double kDestinationMarkerRatio = 0.22;
    const cv::Scalar kCooldownBarBackground(60, 60, 60, 255);
    const cv::Scalar kCooldownBarFill(80, 200, 80, 255);
    const cv::Scalar kSelectionColor(0, 215, 255, 255);

    int pieceMarginFor(int cellSizePixels) {
        return static_cast<int>(std::lround(cellSizePixels * kPieceMarginRatio));
    }

    bool isResting(Piece::State state) {
        return state == Piece::State::ShortRest || state == Piece::State::LongRest;
    }
}

ImageView::ImageView(BoardMapper mapper, const std::filesystem::path& assetsRoot,
                     const std::filesystem::path& boardImagePath)
    : mapper_(mapper),
      pieceMarginPixels_(pieceMarginFor(mapper.cellSizePixels())),
      cache_(assetsRoot, mapper.cellSizePixels() - 2 * pieceMarginPixels_) {
    boardImg_.read(boardImagePath,
                   {mapper_.boardPixelWidth(), mapper_.boardPixelHeight()});
}

void ImageView::render(const GameSnapshot& snapshot) {
    long long dtMs = elapsedSinceLastRenderMs();
    Img frame = boardImg_.clone();
    drawSelectionHighlight(frame);
    drawLegalDestinationMarkers(frame);
    drawPieces(snapshot, frame, dtMs);
    removeViewsOfCapturedPieces(snapshot.board());
    frame.show(kShowWaitMs);
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
