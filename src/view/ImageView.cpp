#include "view/ImageView.h"
#include <set>

namespace {
    constexpr int kShowWaitMs = 1;
}

ImageView::ImageView(BoardMapper mapper, const std::filesystem::path& assetsRoot,
                     const std::filesystem::path& boardImagePath)
    : mapper_(mapper),
      cache_(assetsRoot, mapper.cellSizePixels()) {
    boardImg_.read(boardImagePath.string(),
                   {mapper_.boardPixelWidth(), mapper_.boardPixelHeight()});
}

void ImageView::render(const GameSnapshot& snapshot) {
    long long dtMs = elapsedSinceLastRenderMs();
    Img frame = boardImg_.clone();
    drawPieces(snapshot.board(), frame, dtMs);
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

PieceView& ImageView::viewFor(const Piece& piece) {
    auto found = pieceViews_.find(piece.id());
    if (found == pieceViews_.end() || found->second.kind != piece.kind()) {
        const auto& animations = cache_.animationsFor(piece.color(), piece.kind());
        TrackedPiece tracked{piece.kind(), PieceView(animations, piece.state())};
        found = pieceViews_.insert_or_assign(piece.id(), std::move(tracked)).first;
    }
    return found->second.view;
}

void ImageView::drawPieces(const Board& board, Img& frame, long long dtMs) {
    for (int row = 0; row < board.rowCount(); ++row) {
        for (int col = 0; col < board.colCount(); ++col) {
            const Piece* piece = board.pieceAt(row, col);
            if (!piece) continue;

            PieceView& view = viewFor(*piece);
            view.advance(piece->state(), dtMs);
            PixelPoint pixel = mapper_.topLeftPixelOf(piece->cell());
            view.currentFrame().draw_on(frame, pixel.x, pixel.y);
        }
    }
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
