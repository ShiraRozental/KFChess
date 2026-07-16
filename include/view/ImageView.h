#pragma once
#include <chrono>
#include <filesystem>
#include <map>
#include <optional>
#include "input/BoardMapper.h"
#include "view/AnimationCache.h"
#include "view/Img.h"
#include "view/PieceView.h"
#include "view/Renderer.h"

class ImageView : public Renderer {
public:
    ImageView(BoardMapper mapper, const std::filesystem::path& assetsRoot,
              const std::filesystem::path& boardImagePath);

    void render(const GameSnapshot& snapshot) override;

private:
    struct TrackedPiece {
        PieceType kind;
        PieceView view;
    };

    long long elapsedSinceLastRenderMs();
    PieceView& viewFor(const Piece& piece);
    void drawPieces(const GameSnapshot& snapshot, Img& frame, long long dtMs);
    void drawCooldownBar(Img& frame, PixelPoint cellTopLeft, double progress);
    void removeViewsOfCapturedPieces(const Board& board);

    BoardMapper mapper_;
    int pieceMarginPixels_;
    AnimationCache cache_;
    Img boardImg_;
    std::map<PieceId, TrackedPiece> pieceViews_;
    std::optional<std::chrono::steady_clock::time_point> lastRenderTime_;
};
